#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <chunk.h>
#include <QMainWindow>
#include <QStackedWidget>

#include "chunkmodel.h"
#include "panels/mxhd.h"
#include "panels/panel.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

signals:

private:
  //bool CloseFile();

  void InitializeMenuBar();

  void SetPanel(Panel *panel, Data *data);

  ChunkModel model_;
  Chunk chunk_;

  QStackedWidget *config_stack_;

  Panel *panel_blank_;
  MxHdPanel *panel_mxhd_;

  Data *last_set_data_;

private slots:
  void OpenFile();
  //bool SaveFile();
  //bool SaveFileAs();

  void SelectionChanged(const QModelIndex &index);

};

#endif // MAINWINDOW_H
