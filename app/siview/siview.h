#ifndef SIVIEW_H
#define SIVIEW_H

#include <QDialog>
#include <QStackedWidget>

#include "chunkmodel.h"
#include "panels/mxch.h"
#include "panels/mxhd.h"
#include "panels/mxob.h"
#include "panels/mxof.h"
#include "panels/riff.h"
#include "panel.h"

class SIViewDialog : public QDialog
{
  Q_OBJECT
public:
  enum Mode {
    Import,
    Export
  };

  SIViewDialog(Mode mode, si::Chunk *riff, QWidget *parent = nullptr);

private:
  void SetPanel(Panel *panel, si::Chunk *chunk);

  QStackedWidget *config_stack_;

  ChunkModel chunk_model_;

  Panel *panel_blank_;
  RIFFPanel *panel_riff_;
  MxHdPanel *panel_mxhd_;
  MxChPanel *panel_mxch_;
  MxOfPanel *panel_mxof_;
  MxObPanel *panel_mxob_;

  si::Chunk *last_set_data_;
  si::Chunk *root_;

private slots:
  void SelectionChanged(const QModelIndex &index);

  void ImmediateReweave();

};

#endif // SIVIEW_H
