#include "wavpanel.h"

#include <iostream>
#include <object.h>
#include <QGroupBox>
#include <QHBoxLayout>

WavPanel::WavPanel(QWidget *parent) :
  Panel(parent),
  audio_out_(nullptr)
{
  int row = 0;

  auto wav_group = new QGroupBox(tr("Playback"));
  layout()->addWidget(wav_group, row, 0, 1, 2);

  auto wav_layout = new QHBoxLayout(wav_group);

  playhead_slider_ = new QSlider(Qt::Horizontal);
  playhead_slider_->setMinimum(0);
  connect(playhead_slider_, &QSlider::valueChanged, this, &WavPanel::SliderMoved);
  wav_layout->addWidget(playhead_slider_);

  play_btn_ = new QPushButton(tr("Play"));
  play_btn_->setCheckable(true);
  connect(play_btn_, &QPushButton::clicked, this, &WavPanel::Play);
  wav_layout->addWidget(play_btn_);

  FinishLayout();

  buffer_.setBuffer(&array_);

  playback_timer_ = new QTimer(this);
  playback_timer_->setInterval(100);
  connect(playback_timer_, &QTimer::timeout, this, &WavPanel::TimerUpdate);
}

void WavPanel::OnOpeningData(void *data)
{
  si::Object *o = static_cast<si::Object*>(data);

  // Find fmt and data
  header_ = *o->GetFileHeader().cast<si::WAVFormatHeader>();
  playhead_slider_->setMaximum(o->GetFileBodySize()/GetSampleSize());
}

void WavPanel::OnClosingData(void *data)
{
  Play(false);
  playhead_slider_->setValue(0);
}

int WavPanel::GetSampleSize() const
{
  return (header_.BitsPerSample/8) * header_.Channels;
}

void WavPanel::Play(bool e)
{
  if (audio_out_) {
    audio_out_->stop();
    delete audio_out_;
    audio_out_ = nullptr;
  }
  buffer_.close();
  array_.clear();
  playback_timer_->stop();

  if (e) {
    si::Object *o = static_cast<si::Object*>(GetData());
    si::bytearray pcm = o->GetFileBody();
    array_ = QByteArray(pcm.data(), pcm.size());
    buffer_.open(QBuffer::ReadOnly);

    size_t start = 0;

    if (playhead_slider_->value() < playhead_slider_->maximum()) {
      start += playhead_slider_->value() * GetSampleSize();
    }

    buffer_.seek(start);

    QAudioFormat audio_fmt;
    audio_fmt.setSampleRate(header_.SampleRate);
    audio_fmt.setChannelCount(header_.Channels);
    audio_fmt.setSampleSize(header_.BitsPerSample);
    audio_fmt.setByteOrder(QAudioFormat::LittleEndian);
    audio_fmt.setCodec(QStringLiteral("audio/pcm"));
    audio_fmt.setSampleType(QAudioFormat::SignedInt);

    audio_out_ = new QAudioOutput(audio_fmt, this);
    connect(audio_out_, &QAudioOutput::stateChanged, this, &WavPanel::OutputChanged);
    audio_out_->start(&buffer_);

    playback_timer_->start();
  }
}

void WavPanel::TimerUpdate()
{
  playhead_slider_->setValue(buffer_.pos() / GetSampleSize());
}

void WavPanel::OutputChanged(QAudio::State state)
{
  if (state != QAudio::ActiveState) {
    Play(false);
    play_btn_->setChecked(false);
  }
}

void WavPanel::SliderMoved(int i)
{
  if (buffer_.isOpen()) {
    buffer_.seek(i * GetSampleSize());
  }
}
