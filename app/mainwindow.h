#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <interleaf.h>
#include <object.h>
#include <QGroupBox>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTreeView>

#include "objectmodel.h"
#include "panel.h"
#include "viewer/bitmappanel.h"
#include "viewer/wavpanel.h"

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

  void SetPanel(Panel *panel, si::Object *chunk);

  void ExtractObject(si::Object *obj);

  QStackedWidget *config_stack_;

  QTreeView *tree_;

  QGroupBox *action_grp_;

  Panel *panel_blank_;
  WavPanel *panel_wav_;
  BitmapPanel *panel_bmp_;

  ObjectModel model_;
  si::Interleaf interleaf_;

  si::Object *last_set_data_;

private slots:
  void OpenFile();
  //bool SaveFile();
  //bool SaveFileAs();

  void ExportFile();

  void SelectionChanged(const QModelIndex &index);

  void ShowContextMenu(const QPoint &p);

  void ExtractSelectedItems();

  void ExtractClicked();

};

#endif // MAINWINDOW_H
