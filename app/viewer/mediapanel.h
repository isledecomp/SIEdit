#ifndef MEDIAPANEL_H
#define MEDIAPANEL_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <file.h>
#include <QAudioOutput>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

#include "panel.h"

class MediaPanel : public Panel
{
  Q_OBJECT
public:
  MediaPanel(QWidget *parent = nullptr);
  virtual ~MediaPanel() override;

  qint64 ReadAudio(char *data, qint64 maxlen);

protected:
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  void Close();

  void VideoUpdate(float t);
  void AudioSeek(float t);

  int GetNextFrame(AVCodecContext *cctx, unsigned int stream, AVFrame *frame);

  void ClearQueue();

  void StartAudioPlayback();

  static float SliderValueToFloatSeconds(int i, int max, AVStream *s);

  AVFormatContext *m_FmtCtx;
  AVPacket *m_Packet;
  std::list<AVFrame*> m_FrameQueue;

  AVCodecContext *m_VideoCodecCtx;
  AVStream *m_VideoStream;
  AVFrame *m_SwsFrame;
  AVFrame *m_AudioFrame;

  AVCodecContext *m_AudioCodecCtx;
  AVStream *m_AudioStream;

  SwsContext *m_SwsCtx;
  SwrContext *m_SwrCtx;

  AVIOContext *m_IoCtx;

  si::MemoryBuffer m_Data;

  QLabel *m_ImgViewer;

  QAudioOutput *m_AudioOutput;
  QIODevice *m_AudioNotifyDevice;
  QByteArray m_AudioBuffer;
  AVSampleFormat m_AudioOutputSampleFmt;
  QSlider *m_PlayheadSlider;
  QPushButton *m_PlayBtn;
  QTimer *m_PlaybackTimer;
  qint64 m_PlaybackStart;
  float m_PlaybackOffset;
  bool m_AudioFlushed;
  bool m_SliderPressed;

private slots:
  void Play(bool e);

  void TimerUpdate();

  void SliderPressed();
  void SliderMoved(int i);
  void SliderReleased();

  void AudioStateChanged(QAudio::State state);

};

class MediaAudioDevice : public QIODevice
{
  Q_OBJECT
public:
  MediaAudioDevice(MediaPanel *o = nullptr);

protected:
  virtual qint64 readData(char *data, qint64 maxSize) override;
  virtual qint64 writeData(const char *data, qint64 maxSize) override;

private:
  MediaPanel *m_MediaPanel;

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
