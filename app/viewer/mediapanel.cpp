#include "mediapanel.h"

#include <object.h>
#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QMenu>
#include <QMouseEvent>

MediaPanel::MediaPanel(QWidget *parent) :
  Panel(parent),
  m_AudioOutput(nullptr),
  m_SliderPressed(false),
  m_vflip(false)
{
  int row = 0;

  auto wav_group = new QGroupBox(tr("Playback"));
  layout()->addWidget(wav_group, row, 0, 1, 2);

  auto preview_layout = new QVBoxLayout(wav_group);

  m_ImgViewer = new QLabel();
  m_ImgViewer->setAlignment(Qt::AlignCenter);
  m_ImgViewer->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ImgViewer, &QWidget::customContextMenuRequested, this, &MediaPanel::LabelContextMenuTriggered);
  preview_layout->addWidget(m_ImgViewer);

  auto wav_layout = new QHBoxLayout();
  preview_layout->addLayout(wav_layout);

  m_PlayheadSlider = new ClickableSlider(Qt::Horizontal);
  m_PlayheadSlider->setMinimum(0);
  m_PlayheadSlider->setMaximum(100000);
  connect(m_PlayheadSlider, &QSlider::sliderPressed, this, &MediaPanel::SliderPressed);
  connect(m_PlayheadSlider, &QSlider::sliderMoved, this, &MediaPanel::SliderMoved);
  connect(m_PlayheadSlider, &QSlider::sliderReleased, this, &MediaPanel::SliderReleased);
  wav_layout->addWidget(m_PlayheadSlider);

  m_PlayBtn = new QPushButton(tr("Play"));
  m_PlayBtn->setCheckable(true);
  connect(m_PlayBtn, &QPushButton::clicked, this, &MediaPanel::Play);
  wav_layout->addWidget(m_PlayBtn);

  FinishLayout();

  m_PlaybackTimer = new QTimer(this);
  m_PlaybackTimer->setInterval(10);
  connect(m_PlaybackTimer, &QTimer::timeout, this, &MediaPanel::TimerUpdate);

  m_mediaInstance = new MediaInstance(this);
  connect(m_mediaInstance, &MediaInstance::EndOfFile, this, &MediaPanel::EndOfFile);

  m_AudioNotifyDevice = new MediaAudioDevice(m_mediaInstance, this);
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
    qint64 copy_len = std::min(maxlen, qint64(m_AudioBuffer.size()));
    memcpy(data, m_AudioBuffer.data(), copy_len);
    m_AudioBuffer = m_AudioBuffer.mid(copy_len);
    return copy_len;
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
  si::Object *o = static_cast<si::Object*>(data);

  m_mediaInstance->Open(o->ExtractToMemory());

  if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_VIDEO) {
    // Heuristic to flip phoneme flics vertically
    m_vflip = (o->name().find("_Pho_") != std::string::npos);

    VideoUpdate(0);
  }
}

void MediaPanel::OnClosingData(void *data)
{
  Close();
}

void MediaPanel::Close()
{
  Play(false);

  m_mediaInstance->Close();

  m_PlayheadSlider->setValue(0);

  m_ImgViewer->setPixmap(QPixmap());

  m_vflip = false;
}

QImage MediaInstance::GetVideoFrame(float t)
{
  // Convert percent to duration seconds
  int64_t ts = PercentToTimestamp(t);
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

        if (ret == AVERROR_EOF) {
          emit EndOfFile();
        }
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
  int ret;
  av_frame_unref(frame);
  while ((ret = avcodec_receive_frame(m_CodecCtx, frame)) == AVERROR(EAGAIN)) {
    av_packet_unref(m_Packet);
    ret = av_read_frame(m_FmtCtx, m_Packet);
    if (ret < 0) {
      return ret;
    }

    if (m_Packet->stream_index == m_Stream->index) {
      ret = avcodec_send_packet(m_CodecCtx, m_Packet);
      if (ret < 0) {
        return ret;
      }
    }
  }

  return ret;
}

void MediaPanel::StartAudioPlayback()
{
  auto output_dev = QAudioDeviceInfo::defaultOutputDevice();
  auto fmt = output_dev.preferredFormat();

  if (m_mediaInstance->StartPlayingAudio(output_dev, fmt)) {
    m_AudioOutput = new QAudioOutput(output_dev, fmt, this);
    m_AudioNotifyDevice->open(QIODevice::ReadOnly);
    connect(m_AudioOutput, &QAudioOutput::stateChanged, this, &MediaPanel::AudioStateChanged);
    m_AudioOutput->start(m_AudioNotifyDevice);
  }
}

void MediaPanel::VideoUpdate(float t)
{
  QImage img = m_mediaInstance->GetVideoFrame(t);
  if (!img.isNull()) {
    if (m_vflip) {
      img = img.mirrored(false, true);
    }

    m_ImgViewer->setPixmap(QPixmap::fromImage(img));
  }
}

float MediaPanel::GetRealSliderValue() const
{
  return float(m_PlayheadSlider->value()) / m_PlayheadSlider->maximum();
}

int MediaPanel::GetFakeSliderValueFromReal(float t) const
{
  return t * m_PlayheadSlider->maximum();
}

void MediaPanel::Play(bool e)
{
  if (m_AudioOutput) {
    m_AudioOutput->stop();
    delete m_AudioOutput;
    m_AudioOutput = nullptr;

    m_AudioNotifyDevice->close();
  }

  if (e) {
    if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_VIDEO) {
      m_PlaybackOffset = GetRealSliderValue();
    } else {
      m_PlaybackOffset = 0;
      if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_AUDIO) {
        StartAudioPlayback();
      }
    }

    m_PlaybackStart = QDateTime::currentMSecsSinceEpoch();
    m_PlaybackTimer->start();
  } else {
    m_PlaybackTimer->stop();
  }
  m_PlayBtn->setChecked(e);
}

void MediaPanel::TimerUpdate()
{
  if (!m_SliderPressed && m_mediaInstance->GetStreamPosition() != AV_NOPTS_VALUE) {
    m_PlayheadSlider->setValue(GetFakeSliderValueFromReal(m_mediaInstance->GetTime()));
  }

  if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_VIDEO) {
    float now_seconds = float(QDateTime::currentMSecsSinceEpoch() - m_PlaybackStart) * 0.001f;
    float now = m_mediaInstance->SecondsToPercent(now_seconds);

    VideoUpdate(now + m_PlaybackOffset);
  }
}

void MediaPanel::UpdateVideo()
{
  SliderMoved(m_PlayheadSlider->value());
}

void MediaPanel::SliderPressed()
{
  m_SliderPressed = true;
}

void MediaPanel::SliderMoved(int i)
{
  if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_VIDEO) {
    float f = GetRealSliderValue();
    m_PlaybackOffset = f;
    m_PlaybackStart = QDateTime::currentMSecsSinceEpoch();
    VideoUpdate(f);
  } else if (m_mediaInstance->codec_type() == AVMEDIA_TYPE_AUDIO) {
    m_mediaInstance->Seek(GetRealSliderValue());
  }
}

void MediaPanel::SliderReleased()
{
  m_SliderPressed = false;
}

void MediaPanel::AudioStateChanged(QAudio::State state)
{
  if (state == QAudio::IdleState) {
    Play(false);
    m_PlayheadSlider->setValue(m_PlayheadSlider->maximum());
  }
}

void MediaPanel::LabelContextMenuTriggered(const QPoint &pos)
{
  QMenu m(this);

  auto vert_flip = m.addAction(tr("Flip Vertically"));
  vert_flip->setCheckable(true);
  vert_flip->setChecked(m_vflip);
  connect(vert_flip, &QAction::triggered, this, [this](bool e){
    m_vflip = e;
    UpdateVideo();
  });

  m.exec(static_cast<QWidget*>(sender())->mapToGlobal(pos));
}

void MediaPanel::EndOfFile()
{
  if (IsPlaying()) {
    Play(false);
    if (!m_SliderPressed) {
      m_PlayheadSlider->setValue(m_PlayheadSlider->maximum());
    }
  }
}

MediaAudioDevice::MediaAudioDevice(MediaInstance *o, QObject *parent) :
  QIODevice(parent)
{
  m_mediaInstance = o;
}

qint64 MediaAudioDevice::readData(char *data, qint64 maxSize)
{
  return m_mediaInstance->ReadAudio(data, maxSize);
}

qint64 MediaAudioDevice::writeData(const char *data, qint64 maxSize)
{
  return -1;
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
  m_IoCtx(nullptr)
{

}

void MediaInstance::Open(const si::MemoryBuffer &buf)
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

bool MediaInstance::StartPlayingAudio(const QAudioDeviceInfo &output_dev, const QAudioFormat &fmt)
{
  AVSampleFormat smp_fmt = AV_SAMPLE_FMT_S16;
  switch (fmt.sampleType()) {
  case QAudioFormat::Unknown:
    break;
  case QAudioFormat::SignedInt:
    switch (fmt.sampleSize()) {
    case 16:
      smp_fmt = AV_SAMPLE_FMT_S16;
      break;
    case 32:
      smp_fmt = AV_SAMPLE_FMT_S32;
      break;
    case 64:
      smp_fmt = AV_SAMPLE_FMT_S64;
      break;
    }
    break;
  case QAudioFormat::UnSignedInt:
    switch (fmt.sampleSize()) {
    case 8:
      smp_fmt = AV_SAMPLE_FMT_U8;
      break;
    }
    break;
  case QAudioFormat::Float:
    switch (fmt.sampleSize()) {
    case 32:
      smp_fmt = AV_SAMPLE_FMT_FLT;
      break;
    case 64:
      smp_fmt = AV_SAMPLE_FMT_DBL;
      break;
    }
    break;
  }

  m_playbackFormat = fmt;
  m_AudioOutputSampleFmt = smp_fmt;

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
  } else {
    if (swr_init(m_SwrCtx) < 0) {
      qCritical() << "Failed to init swr ctx";
    } else {
      if (m_AudioFlushed) {
        Seek(0);
        m_AudioFlushed = false;
      }
      m_AudioBuffer.clear();

      return true;
    }
  }

  return false;
}

void MediaInstance::Seek(float t)
{
  av_seek_frame(m_FmtCtx, m_Stream->index, PercentToTimestamp(t), AVSEEK_FLAG_BACKWARD);
}

void MediaInstance::ClearQueue()
{
  while (!m_FrameQueue.empty()) {
    av_frame_free(&m_FrameQueue.front());
    m_FrameQueue.pop_front();
  }
}

float MediaInstance::PercentToSeconds(float t) const
{
  return t * (float(m_Stream->time_base.num) * float(m_Stream->duration) / float(m_Stream->time_base.den));
}

float MediaInstance::SecondsToPercent(float t) const
{
  return t / (float(m_Stream->time_base.num) * float(m_Stream->duration) / float(m_Stream->time_base.den));
}

int64_t MediaInstance::PercentToTimestamp(float t) const
{
  return std::floor(t * float(m_Stream->duration));
}
