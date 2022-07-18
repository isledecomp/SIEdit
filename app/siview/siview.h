#ifndef SIVIEW_H
#define SIVIEW_H

#include <interleaf.h>
#include <QDialog>
#include <QStackedWidget>

#include "chunkmodel.h"
#include "infopanel.h"

class SIViewDialog : public QWidget
{
  Q_OBJECT
public:
  SIViewDialog(si::Info *info, QWidget *parent = nullptr);

  std::unique_ptr<si::Interleaf> temp;

private:
  QStackedWidget *config_stack_;

  ChunkModel chunk_model_;

  InfoPanel *panel_;

  const si::Info *last_set_data_;
  const si::Info *root_;

  std::unique_ptr<si::Interleaf> temp_interleaf_;

private slots:
  void SelectionChanged(const QModelIndex &index);

};

#endif // SIVIEW_H
