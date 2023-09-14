#include "siview.h"

#include <QFileDialog>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

using namespace si;

SIViewDialog::SIViewDialog(Info *riff, QWidget *parent) :
  QWidget(parent, Qt::Window),
  root_(riff),
  last_set_data_(nullptr)
{
  auto layout = new QVBoxLayout(this);

  auto splitter = new QSplitter();
  splitter->setChildrenCollapsible(false);
  layout->addWidget(splitter);

  auto tree = new QTreeView();
  chunk_model_.SetCore(riff);
  tree->setModel(&chunk_model_);
  tree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SIViewDialog::SelectionChanged);
  splitter->addWidget(tree);

  /*config_stack_ = new QStackedWidget();
  config_stack_->setContentsMargins(0,0,0,0);
  splitter->addWidget(config_stack_);

  panel_ = new InfoPanel();
  config_stack_->addWidget(panel_);*/

  panel_ = new InfoPanel();
  splitter->addWidget(panel_);

  splitter->setSizes({99999, 99999});

  SetSubtitle(QString());
}

void SIViewDialog::SetSubtitle(const QString &s)
{
  QString t;
  if (s.isEmpty()) {
    t = tr("View SI File");
  } else {
    t = tr("View SI File: %1").arg(s);
  }
  setWindowTitle(t);
}

void SIViewDialog::SelectionChanged(const QModelIndex &index)
{
  panel_->SetData(static_cast<Info*>(index.internalPointer()));
}
