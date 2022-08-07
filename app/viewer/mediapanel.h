#ifndef MEDIAPANEL_H
#define MEDIAPANEL_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <file.h>
#include <object.h>
#include <QAudioOutput>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

#include "panel.h"

class MediaInstance : public QObject
{
  Q_OBJECT
public:
  MediaInstance(QObject *parent = nullptr);

  void Open(const si::bytearray &buf);

  void Close();

  AVMediaType codec_type() const
  {
    return m_Stream ? m_Stream->codecpar->codec_type : AVMEDIA_TYPE_UNKNOWN;
  }

  bool StartPlayingAudio(const QAudioDeviceInfo &output_dev, const QAudioFormat &fmt);

  void Seek(float t);

  int GetNextFrame(AVFrame *frame);

  qint64 ReadAudio(char *data, qint64 maxlen);

  QImage GetVideoFrame(float f);

  float GetTime() const
  {
    return float(m_Frame->pts) / m_duration;
  }

  float PercentToSeconds(float t) const;
  float SecondsToPercent(float t) const;

  int64_t PercentToTimestamp(float t) const;

  bool IsEndOfFile()
  {
    return m_eof;
  }

  void ResetEOF()
  {
    m_eof = false;
  }

signals:
  void EndOfFile();

private:
  void ClearQueue();

  AVFormatContext *m_FmtCtx;
  AVStream *m_Stream;
  std::list<AVFrame*> m_FrameQueue;

  AVPacket *m_Packet;
  AVFrame *m_Frame;

  AVCodecContext *m_CodecCtx;

  SwsContext *m_SwsCtx;
  SwrContext *m_SwrCtx;

  bool m_AudioFlushed;
  QByteArray m_AudioBuffer;

  AVIOContext *m_IoCtx;

  si::MemoryBuffer m_Data;

  QAudioFormat m_playbackFormat;
  AVSampleFormat m_AudioOutputSampleFmt;

  bool m_eof;

  int64_t m_duration;

};

class MediaPanel : public Panel
{
  Q_OBJECT
public:
  MediaPanel(QWidget *parent = nullptr);
  virtual ~MediaPanel() override;

  bool IsPlaying() const
  {
    return m_PlaybackTimer->isActive();
  }

  const std::vector<MediaInstance *> &GetMediaInstances() const
  {
    return m_mediaInstances;
  }

protected:
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  void Close();

  void StartAudioPlayback();

  void VideoUpdate(float t);

  float GetRealSliderValue() const;
  int GetFakeSliderValueFromReal(float t) const;

  void OpenMediaInstance(si::Object *o);

  std::vector<QLabel *> m_imgViewers;
  std::vector<MediaInstance *> m_mediaInstances;

  QAudioOutput *m_AudioOutput;
  QSlider *m_PlayheadSlider;
  QPushButton *m_PlayBtn;
  QTimer *m_PlaybackTimer;
  qint64 m_PlaybackStart;
  float m_PlaybackOffset;
  bool m_SliderPressed;
  QVBoxLayout *m_viewerLayout;

private slots:
  void Play(bool e);

  void TimerUpdate();

  void UpdateVideo();

  void SliderPressed();
  void SliderMoved(int i);
  void SliderReleased();

  void AudioStateChanged(QAudio::State state);

  void LabelContextMenuTriggered(const QPoint &pos);

};

class MediaAudioDevice : public QIODevice
{
  Q_OBJECT
public:
  MediaAudioDevice(MediaPanel *panel, QAudioFormat::SampleType type, QObject *parent);

protected:
  virtual qint64 readData(char *data, qint64 maxSize) override;
  virtual qint64 writeData(const char *data, qint64 maxSize) override;

private:
  MediaPanel *m_mediaPanel;
  QAudioFormat::SampleType m_sampleType;

};

class ClickableSlider : public QSlider
{
  Q_OBJECT
public:
  ClickableSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
  ClickableSlider(QWidget *parent = nullptr);

protected:
  virtual void mousePressEvent(QMouseEvent *e) override;

};

#endif // MEDIAPANEL_H
