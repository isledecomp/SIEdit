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
  void InitializeMenuBar();

  void SetPanel(Panel *panel, si::Object *chunk);

  void ExtractObject(si::Object *obj);
  void ReplaceObject(si::Object *obj);

  static bool OpenInterleafFileInternal(QWidget *parent, si::Interleaf *interleaf, const QString &s);

  QString GetOpenFileName();

  static const QString kFileFilter;

  QStackedWidget *config_stack_;

  QTreeView *tree_;

  QGroupBox *action_grp_;

  Panel *panel_blank_;
  WavPanel *panel_wav_;
  BitmapPanel *panel_bmp_;

  ObjectModel model_;
  si::Interleaf interleaf_;

  si::Object *last_set_data_;

  QString current_filename_;

private slots:
  void NewFile();
  void OpenFile();
  bool SaveFile();
  bool SaveFileAs();

  void SelectionChanged(const QModelIndex &index);

  void ShowContextMenu(const QPoint &p);

  void ExtractSelectedItems();
  void ExtractClicked();

  void ReplaceClicked();

  void ViewSIFile();

};

#endif // MAINWINDOW_H
