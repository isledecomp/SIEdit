#include "mediapanel.h"

#include <othertypes.h>

#include <QAudioSink>
#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QMediaDevices>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollArea>

MediaPanel::MediaPanel(QWidget *parent) :
  Panel(parent)
{
  int row = 0;

  auto wav_group = new QGroupBox(tr("Playback"));
  layout()->addWidget(wav_group, row, 0, 1, 2);

  auto preview_layout = new QVBoxLayout(wav_group);

  auto viewer_scroll = new QScrollArea();
  viewer_scroll->setWidgetResizable(true);

  auto viewer_inner = new QWidget();
  viewer_scroll->setWidget(viewer_inner);

  preview_layout->addWidget(viewer_scroll, 1);

  m_viewerLayout = new QVBoxLayout(viewer_inner);
  m_viewerLayout->setContentsMargins(0, 0, 0, 0);

  auto ctrl_layout = new QHBoxLayout();
  preview_layout->addLayout(ctrl_layout);

  m_PlayheadSlider = new ClickableSlider(Qt::Horizontal);
  m_PlayheadSlider->setMinimum(0);
  connect(m_PlayheadSlider, &QSlider::sliderPressed, this, &MediaPanel::SliderPressed);
  connect(m_PlayheadSlider, &QSlider::sliderMoved, this, &MediaPanel::SliderMoved);
  connect(m_PlayheadSlider, &QSlider::sliderReleased, this, &MediaPanel::SliderReleased);
  ctrl_layout->addWidget(m_PlayheadSlider);

  m_PlayBtn = new QPushButton(tr("Play"));
  m_PlayBtn->setCheckable(true);
  connect(m_PlayBtn, &QPushButton::clicked, this, &MediaPanel::Play);
  ctrl_layout->addWidget(m_PlayBtn);

  //FinishLayout();

  m_PlaybackTimer = new QTimer(this);
  m_PlaybackTimer->setInterval(10);
  connect(m_PlaybackTimer, &QTimer::timeout, this, &MediaPanel::TimerUpdate);
}

MediaPanel::~MediaPanel()
{
  Close();
}

qint64 MediaInstance::ReadAudio(char *data, qint64 maxlen)
{
  if (m_AudioFlushed) {
    return 0;
  }

  qint64 dest_start = 0;

  if (m_virtualPosition < 0) {
    int64_t silent_bytes = SecondsToBytes(-m_virtualPosition);
    if (silent_bytes >= maxlen) {
      memset(data, 0, maxlen);
      m_virtualPosition += BytesToSeconds(maxlen);
      return maxlen;
    } else {
      memset(data, 0, silent_bytes);
      dest_start += silent_bytes;
      m_virtualPosition = 0;
    }
  }

  while (!m_AudioFlushed && m_AudioBuffer.size() < maxlen) {
    int ret = GetNextFrame(m_Frame);
    if (ret >= 0 || ret == AVERROR_EOF) {
      const uint8_t **in_data;
      int in_nb_samples;
      if (ret == AVERROR_EOF) {
        in_data = nullptr;
        in_nb_samples = 0;
        m_AudioFlushed = true;
      } else {
        in_data = const_cast<const uint8_t**>(m_Frame->data);
        in_nb_samples = m_Frame->nb_samples;
      }

      int dst_nb_samples = av_rescale_rnd(swr_get_delay(m_SwrCtx, m_Stream->codecpar->sample_rate) + in_nb_samples,
                                          m_playbackFormat.sampleRate(), m_Stream->codecpar->sample_rate, AV_ROUND_UP);
      int data_size = dst_nb_samples * av_get_bytes_per_sample(m_AudioOutputSampleFmt) * m_playbackFormat.channelCount();

      int old_sz = m_AudioBuffer.size();
      m_AudioBuffer.resize(old_sz + data_size);

      uint8_t *out = reinterpret_cast<uint8_t*>(m_AudioBuffer.data() + old_sz);
      int converted = swr_convert(m_SwrCtx, &out, dst_nb_samples, in_data, in_nb_samples);

      data_size = converted * av_get_bytes_per_sample(m_AudioOutputSampleFmt) * m_playbackFormat.channelCount();

      if (m_AudioBuffer.size() != old_sz + data_size) {
        m_AudioBuffer.resize(old_sz + data_size);
      }
    } else {
      break;
    }
  }

  if (!m_AudioBuffer.isEmpty()) {
    qint64 copy_len = std::min(maxlen - dest_start, qint64(m_AudioBuffer.size()));
    memcpy(data + dest_start, m_AudioBuffer.data(), copy_len);
    m_AudioBuffer = m_AudioBuffer.mid(copy_len);

    qint64 total = copy_len + dest_start;
    m_virtualPosition += BytesToSeconds(total);
    return total;
  }

  return 0;
}

int ReadData(void *opaque, uint8_t *buf, int buf_sz)
{
  si::MemoryBuffer *m = static_cast<si::MemoryBuffer *>(opaque);

  int s = m->ReadData(reinterpret_cast<char*>(buf), buf_sz);
  if (s == 0) {
    if (m->pos() == m->size()) {
      s = AVERROR_EOF;
    }
  }

  return s;
}

int64_t SeekData(void *opaque, int64_t offset, int whence)
{
  si::MemoryBuffer *m = static_cast<si::MemoryBuffer *>(opaque);

  if (whence == AVSEEK_SIZE) {
    return m->size();
  }

  m->seek(offset);

  return m->pos();
}

void MediaPanel::OnOpeningData(void *data)
{
  OpenMediaInstance(static_cast<si::Object*>(data));

  float total_duration = 0.0f;
  for (auto it=m_mediaInstances.cbegin(); it!=m_mediaInstances.cend(); it++) {
    auto m = *it;
    total_duration = std::max(total_duration, m->GetDuration() + m->GetStartOffset());
  }
  m_PlayheadSlider->setMaximum(GetSliderValueForSeconds(total_duration));
}

void MediaPanel::OnClosingData(void *data)
{
  Close();
}

void MediaPanel::Close()
{
  Play(false);

  qDeleteAll(m_mediaInstances);
  m_mediaInstances.clear();

  m_PlayheadSlider->setValue(0);

  qDeleteAll(m_imgViewers);
  m_imgViewers.clear();
}

QImage MediaInstance::GetVideoFrame(float t)
{
  // Convert percent to duration seconds
  t -= m_startOffset;
  if (t < 0) {
    t = 0;
  }

  int64_t ts = SecondsToTimestamp(t);
  //int64_t second = std::ceil(flipped);

  AVFrame *using_frame = nullptr;
  for (auto it=m_FrameQueue.begin(); it!=m_FrameQueue.end(); it++) {
    auto next = it;
    next++;

    if ((*it)->pts == ts
        || (next != m_FrameQueue.end() && (*next)->pts > ts)) {
      using_frame = *it;
      break;
    }
  }

  if (!using_frame) {
    // Determine if the queue will eventually get this frame
    if (m_FrameQueue.empty()
        //|| ts > m_FrameQueue.back()->pts + second
        || ts < m_FrameQueue.front()->pts) {
      ClearQueue();
      av_seek_frame(m_FmtCtx, m_Stream->index, ts, AVSEEK_FLAG_BACKWARD);
    }

    while (m_FrameQueue.empty() || m_FrameQueue.back()->pts < ts) {
      AVFrame *f = av_frame_alloc();
      int ret = GetNextFrame(f);
      if (ret < 0) {
        av_frame_free(&f);
        break;
      } else {
        AVFrame *previous = nullptr;
        if (!m_FrameQueue.empty()) {
          previous = m_FrameQueue.back();
        }

        m_FrameQueue.push_back(f);

        if (previous && f->pts > ts) {
          using_frame = previous;
          break;
        } else if (f->pts == ts) {
          using_frame = f;
          break;
        }
      }

    }
  }

  if (using_frame) {
    if (using_frame->pts != m_Frame->pts) {
      m_Frame->pts = using_frame->pts;

      sws_scale(m_SwsCtx, using_frame->data, using_frame->linesize, 0, using_frame->height,
                m_Frame->data, m_Frame->linesize);

      return QImage(m_Frame->data[0], m_Frame->width, m_Frame->height, m_Frame->linesize[0], QImage::Format_RGBA8888);
    }
  }

  return QImage();
}

int MediaInstance::GetNextFrame(AVFrame *frame)
{
  m_eof = false;
  int ret;
  av_frame_unref(frame);
  while ((ret = avcodec_receive_frame(m_CodecCtx, frame)) == AVERROR(EAGAIN)) {
    av_packet_unref(m_Packet);
    ret = av_read_frame(m_FmtCtx, m_Packet);
    if (ret < 0) {
      break;
    }

    if (m_Packet->stream_index == m_Stream->index) {
      ret = avcodec_send_packet(m_CodecCtx, m_Packet);
      if (ret < 0) {
        break;
      }
    }
  }

  if (ret == AVERROR_EOF) {
    m_eof = true;
    emit EndOfFile();
  }

  return ret;
}

void MediaPanel::VideoUpdate(float t)
{
  for (size_t i=0; i<m_mediaInstances.size(); i++) {
    auto m = m_mediaInstances.at(i);

    if (m->codec_type() == AVMEDIA_TYPE_VIDEO) {
      QImage img = m->GetVideoFrame(t);
      if (!img.isNull()) {
        auto v = m_imgViewers.at(i);

        if (v->property("vflip").toBool()) {
          img = img.mirrored(false, true);
        }

        v->setPixmap(QPixmap::fromImage(img));
      }
    }
  }
}

float MediaPanel::GetSecondsFromSlider() const
{
  return float(m_PlayheadSlider->value()) / SECONDS_INTERVAL;
}

void MediaPanel::SetSecondsOnSlider(float s)
{
  m_PlayheadSlider->setValue(GetSliderValueForSeconds(s));
}

int MediaPanel::GetSliderValueForSeconds(float s)
{
  return s * SECONDS_INTERVAL;
}

void MediaPanel::OpenMediaInstance(si::Object *o)
{
  switch (o->type()) {
  case si::MxOb::Presenter:
    for (auto it=o->GetChildren().cbegin(); it!=o->GetChildren().cend(); it++) {
      OpenMediaInstance(static_cast<si::Object*>(*it));
    }
    break;
  case si::MxOb::Video:
  case si::MxOb::Sound:
  case si::MxOb::Bitmap:
  {
    auto m = new MediaInstance(this);

    m->Open(o->ExtractToMemory());
    m->SetStartOffset(float(o->time_offset_) * 0.001f);
    m->SetVolume(float(o->volume_) / si::MxOb::MAXIMUM_VOLUME);
    m->SetVirtualTime(0);

    m_mediaInstances.push_back(m);

    if (m->codec_type() == AVMEDIA_TYPE_VIDEO) {
      // Heuristic to flip phoneme flics vertically
      if (m_imgViewers.size() < m_mediaInstances.size()) {
        m_imgViewers.resize(m_mediaInstances.size());
      }

      auto v = new QLabel();
      v->setAlignment(Qt::AlignCenter);
      v->setContextMenuPolicy(Qt::CustomContextMenu);
      v->setProperty("vflip", (o->name().find("_Pho_") != std::string::npos));
      connect(v, &QWidget::customContextMenuRequested, this, &MediaPanel::LabelContextMenuTriggered);
      m_viewerLayout->addWidget(v);
      m_imgViewers[m_mediaInstances.size()-1] = v;

      VideoUpdate(0);
    }
    break;
  }
  case si::MxOb::Null:
  case si::MxOb::World:
  case si::MxOb::Event:
  case si::MxOb::Animation:
  case si::MxOb::Object:
  case si::MxOb::TYPE_COUNT:
    // Do nothing
    break;
  }
}

void MediaPanel::Play(bool e)
{
  {
    // No matter what, stop any current audio
    std::vector<QAudioSink*> copy = m_audioSinks;
    for (auto it=copy.cbegin(); it!=copy.cend(); it++) {
      auto o = *it;
      o->stop();
    }
  }

  if (e) {
    bool has_video = false;
    bool has_audio = false;

    if (m_PlayheadSlider->value() == m_PlayheadSlider->maximum()) {
      m_PlayheadSlider->setValue(0);
      SliderMoved(0);
    }

    m_PlaybackOffset = GetSecondsFromSlider();

    auto output_dev = QAudioDevice(QMediaDevices::defaultAudioOutput());
    auto fmt = output_dev.preferredFormat();

    ClearAudioSinks();
    
    for (auto it=m_mediaInstances.cbegin(); it!=m_mediaInstances.cend(); it++) {
      auto m = *it;

      m->ResetEOF();

      if (m->codec_type() == AVMEDIA_TYPE_VIDEO) {
        has_video = true;
      } else if (m->codec_type() == AVMEDIA_TYPE_AUDIO) {
        if (m_PlaybackOffset < (m->GetDuration() + m->GetStartOffset())) {
          if (m->StartPlayingAudio(output_dev, fmt)) {
            auto out = new QAudioSink(output_dev, fmt, this);
            out->setVolume(m->GetVolume());
            out->start(m);
            m_audioSinks.push_back(out);
            has_audio = true;
          }
        } else {
          m->ResetEOF(true);
        }
      }
    }

    m_PlaybackStart = QDateTime::currentMSecsSinceEpoch();
    m_PlaybackTimer->start();
    m_PlayBtn->setText("Pause");
  } else {
    m_PlayBtn->setText("Play");
    m_PlaybackTimer->stop();
  }
  m_PlayBtn->setChecked(e);
}

void MediaPanel::TimerUpdate()
{
  bool all_eof = true;

  // Don't set slider if slider pressed
  float now = float(QDateTime::currentMSecsSinceEpoch() - m_PlaybackStart) * 0.001f;
  now += m_PlaybackOffset;
  SetSecondsOnSlider(now);

  for (size_t i=0; i<m_mediaInstances.size(); i++) {
    auto m = m_mediaInstances.at(i);

    if (all_eof && !m->IsEndOfFile()) {
      all_eof = false;
    }

    if (m->codec_type() == AVMEDIA_TYPE_VIDEO) {
      VideoUpdate(now);
    } else if (m->codec_type() == AVMEDIA_TYPE_AUDIO) {
      // Do nothing yet
    }
  }

  if (all_eof) {
    ClearAudioSinks();
    Play(false);
    m_PlayheadSlider->setValue(m_PlayheadSlider->maximum());
  }
}

void MediaPanel::UpdateVideo()
{
  SliderMoved(m_PlayheadSlider->value());
}

void MediaPanel::SliderPressed()
{
  if (IsPlaying()) {
    Play(false);
  }
}

void MediaPanel::SliderMoved(int i)
{
  float f = GetSecondsFromSlider();
  m_PlaybackOffset = f;
  m_PlaybackStart = QDateTime::currentMSecsSinceEpoch();

  for (auto it=m_mediaInstances.cbegin(); it!=m_mediaInstances.cend(); it++) {
    auto m = *it;
    if (m->codec_type() == AVMEDIA_TYPE_VIDEO) {
      VideoUpdate(f);
    } else if (m->codec_type() == AVMEDIA_TYPE_AUDIO) {
      m->Seek(f);
    }
  }
}

void MediaPanel::SliderReleased()
{
}

void MediaPanel::LabelContextMenuTriggered(const QPoint &pos)
{
  QMenu m(this);

  QObject *s = sender();

  auto vert_flip = m.addAction(tr("Flip Vertically"));
  vert_flip->setCheckable(true);
  vert_flip->setChecked(s->property("vflip").toBool());
  connect(vert_flip, &QAction::triggered, this, [this, s](bool e){
    s->setProperty("vflip", e);
    UpdateVideo();
  });

  m.exec(static_cast<QWidget*>(sender())->mapToGlobal(pos));
}

void MediaPanel::ClearAudioSinks()
{
  if (m_audioSinks.size() != 0) {
      for (auto s : m_audioSinks)
        delete s;

    m_audioSinks.clear();
  }
}

qint64 MediaInstance::readData(char *data, qint64 maxSize)
{
  return ReadAudio(data, maxSize);
}

qint64 MediaInstance::writeData(const char *data, qint64 maxSize)
{
  return -1;
}

qint64 MediaInstance::size() const
{
  return SecondsToBytes(m_startOffset + GetDuration());
}

ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget *parent) :
  QSlider(orientation, parent)
{
}

ClickableSlider::ClickableSlider(QWidget *parent) :
  QSlider(parent)
{
}

void ClickableSlider::mousePressEvent(QMouseEvent *e)
{
  int v = double(e->pos().x()) / double(width()) * this->maximum();
  setValue(v);
  emit sliderMoved(v);

  QSlider::mousePressEvent(e);
}

MediaInstance::MediaInstance(QObject *parent) :
  m_FmtCtx(nullptr),
  m_Packet(nullptr),
  m_CodecCtx(nullptr),
  m_Stream(nullptr),
  m_Frame(nullptr),
  m_SwsCtx(nullptr),
  m_SwrCtx(nullptr),
  m_IoCtx(nullptr),
  m_startOffset(0.0f)
{
  this->open(QIODevice::ReadOnly);
}

void MediaInstance::Open(const si::bytearray &buf)
{
  static const size_t buf_sz = 4096;

  m_Data = buf;

  m_IoCtx = avio_alloc_context(
        (unsigned char *) av_malloc(buf_sz),
        buf_sz,
        0,
        &m_Data,
        ReadData,
        nullptr,
        SeekData
      );

  m_FmtCtx = avformat_alloc_context();
  m_FmtCtx->pb = m_IoCtx;
  m_FmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

  if (avformat_open_input(&m_FmtCtx, "", nullptr, nullptr) < 0) {
    qCritical() << "Failed to open format context";
    Close();
    return;
  }

  if (avformat_find_stream_info(m_FmtCtx, nullptr) < 0) {
    qCritical() << "Failed to find stream info";
    Close();
    return;
  }

  if (m_FmtCtx->nb_streams == 0) {
    qWarning() << "No streams in file";
    Close();
    return;
  }

  m_Stream = m_FmtCtx->streams[0];

  m_duration = m_Stream->duration;
  if (m_Stream->codecpar->codec_id == AV_CODEC_ID_FLIC) {
    // FFmpeg can't retrieve the FLIC duration, but we can
    si::FLIC *flic = (si::FLIC *) buf.data();
    m_duration = flic->frames;
  }

  const AVCodec *decoder = avcodec_find_decoder(m_Stream->codecpar->codec_id);
  if (!decoder) {
    qWarning() << "Failed to find decoder for type" << avcodec_get_name(m_Stream->codecpar->codec_id);
    Close();
    return;
  }

  m_CodecCtx = avcodec_alloc_context3(decoder);
  avcodec_parameters_to_context(m_CodecCtx, m_Stream->codecpar);
  avcodec_open2(m_CodecCtx, decoder, nullptr);

  m_Packet = av_packet_alloc();
  m_Frame = av_frame_alloc();

  if (m_Stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
    const AVPixelFormat dest = AV_PIX_FMT_RGBA;
    m_SwsCtx = sws_getContext(m_Stream->codecpar->width,
                              m_Stream->codecpar->height,
                              static_cast<AVPixelFormat>(m_Stream->codecpar->format),
                              m_Stream->codecpar->width,
                              m_Stream->codecpar->height,
                              dest,
                              0,
                              nullptr,
                              nullptr,
                              nullptr);

    m_Frame->width = m_Stream->codecpar->width;
    m_Frame->height = m_Stream->codecpar->height;
    m_Frame->format = dest;
    av_frame_get_buffer(m_Frame, 0);
  }
}

void MediaInstance::Close()
{
  if (m_CodecCtx) {
    avcodec_free_context(&m_CodecCtx);
  }

  if (m_SwsCtx) {
    sws_freeContext(m_SwsCtx);
    m_SwsCtx = nullptr;
  }

  if (m_SwrCtx) {
    swr_free(&m_SwrCtx);
  }

  if (m_Packet) {
    av_packet_free(&m_Packet);
  }

  if (m_Frame) {
    av_frame_free(&m_Frame);
  }

  ClearQueue();

  if (m_FmtCtx) {
    avformat_free_context(m_FmtCtx);
    m_FmtCtx = nullptr;
  }

  if (m_IoCtx) {
    avio_context_free(&m_IoCtx);
    m_IoCtx = nullptr;
  }

  m_Stream = nullptr;

  m_Data.Close();
}

bool MediaInstance::StartPlayingAudio(const QAudioDevice &output_dev, const QAudioFormat &fmt)
{
  if (m_SwrCtx) {
    swr_free(&m_SwrCtx);
    m_SwrCtx = nullptr;
  }

  AVSampleFormat smp_fmt = AV_SAMPLE_FMT_S16;
  switch (fmt.sampleFormat()) {
  default:
  case QAudioFormat::Unknown:
    break;
  case QAudioFormat::Int16:
    smp_fmt = AV_SAMPLE_FMT_S16;
    break;
  case QAudioFormat::Int32:
    smp_fmt = AV_SAMPLE_FMT_S32;
    break;
  case QAudioFormat::UInt8:
    smp_fmt = AV_SAMPLE_FMT_U8;
    break;
  case QAudioFormat::Float:
    smp_fmt = AV_SAMPLE_FMT_FLT;
    break;
  }

  m_playbackFormat = fmt;
  m_AudioOutputSampleFmt = smp_fmt;

#if LIBSWRESAMPLE_VERSION_INT >= AV_VERSION_INT(4, 7, 0)
  AVChannelLayout out;
  av_channel_layout_default(&out, fmt.channelCount());

  int r = swr_alloc_set_opts2(&m_SwrCtx,
                              &out,
                              smp_fmt,
                              fmt.sampleRate(),
                              &m_Stream->codecpar->ch_layout,
                              static_cast<AVSampleFormat>(m_Stream->codecpar->format),
                              m_Stream->codecpar->sample_rate,
                              0, nullptr);
  if (r < 0) {
    qCritical() << "Failed to alloc swr ctx:" << r;
#else
  m_SwrCtx = swr_alloc_set_opts(nullptr,
                                av_get_default_channel_layout(fmt.channelCount()),
                                smp_fmt,
                                fmt.sampleRate(),
                                av_get_default_channel_layout(m_Stream->codecpar->channels),
                                static_cast<AVSampleFormat>(m_Stream->codecpar->format),
                                m_Stream->codecpar->sample_rate,
                                0, nullptr);
  if (!m_SwrCtx) {
    qCritical() << "Failed to alloc swr ctx";
#endif
  } else {
    if (swr_init(m_SwrCtx) < 0) {
      qCritical() << "Failed to init swr ctx";
    } else {
      m_AudioFlushed = false;
      m_AudioBuffer.clear();

      return true;
    }
  }

  return false;
}

void MediaInstance::Seek(float seconds)
{
  SetVirtualTime(seconds);
  av_seek_frame(m_FmtCtx, m_Stream->index, SecondsToTimestamp(std::max(0.0f, m_virtualPosition)), AVSEEK_FLAG_BACKWARD);
}

void MediaInstance::ClearQueue()
{
  while (!m_FrameQueue.empty()) {
    av_frame_free(&m_FrameQueue.front());
    m_FrameQueue.pop_front();
  }
}

int64_t MediaInstance::SecondsToTimestamp(float t) const
{
  return t / (float(m_Stream->time_base.num) / float(m_Stream->time_base.den));
}

float MediaInstance::TimestampToSeconds(int64_t t) const
{
  return t * (float(m_Stream->time_base.num) / float(m_Stream->time_base.den));
}

int64_t MediaInstance::SecondsToBytes(float t) const
{
  return std::floor(t * m_playbackFormat.sampleRate()) * m_playbackFormat.bytesPerFrame();
}

float MediaInstance::BytesToSeconds(int64_t t)
{
  return float(t / m_playbackFormat.bytesPerFrame()) / m_playbackFormat.sampleRate();
}

void MediaInstance::SetVirtualTime(float f)
{
  m_virtualPosition = f - m_startOffset;
  seek(SecondsToBytes(f));
}
