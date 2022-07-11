#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <chunk.h>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTreeView>

#include "panel.h"

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

  QStackedWidget *config_stack_;

  QTreeView *tree_;

  Panel *panel_blank_;

  si::Chunk *last_set_data_;

private slots:
  void OpenFile();
  //bool SaveFile();
  //bool SaveFileAs();

  void SelectionChanged(const QModelIndex &index);

  void ShowContextMenu(const QPoint &p);

  void ExtractSelectedItems();

};

#endif // MAINWINDOW_H
