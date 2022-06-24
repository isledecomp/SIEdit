#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <chunk.h>
#include <QMainWindow>

#include "chunkmodel.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);

signals:

private:
  //bool CloseFile();

  void InitializeMenuBar();

  ChunkModel model_;
  Chunk chunk_;

private slots:
  void OpenFile();
  //bool SaveFile();
  //bool SaveFileAs();

};

#endif // MAINWINDOW_H
