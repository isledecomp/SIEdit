#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <chunk.h>
#include <QMainWindow>
#include <QStackedWidget>

#include "chunkmodel.h"
#include "panels/mxch.h"
#include "panels/mxhd.h"
#include "panels/mxob.h"
#include "panels/mxof.h"
#include "panels/riff.h"
#include "panels/panel.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

  void OpenFilename(const QString &s);

signals:

private:
  //bool CloseFile();

  void InitializeMenuBar();

  void SetPanel(Panel *panel, si::Chunk *chunk);

  ChunkModel model_;
  si::Chunk chunk_;

  QStackedWidget *config_stack_;

  Panel *panel_blank_;
  RIFFPanel *panel_riff_;
  MxHdPanel *panel_mxhd_;
  MxChPanel *panel_mxch_;
  MxOfPanel *panel_mxof_;
  MxObPanel *panel_mxob_;

  si::Chunk *last_set_data_;

private slots:
  void OpenFile();
  //bool SaveFile();
  //bool SaveFileAs();

  void SelectionChanged(const QModelIndex &index);

};

#endif // MAINWINDOW_H
