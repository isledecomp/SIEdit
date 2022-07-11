#include "siview.h"

#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QVBoxLayout>

using namespace si;

SIViewDialog::SIViewDialog(Mode mode, Chunk *riff, QWidget *parent) :
  QDialog(parent),
  last_set_data_(nullptr)
{
  setWindowTitle(mode == Import ? tr("Import SI File") : tr("Export SI File"));

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

  config_stack_ = new QStackedWidget();
  splitter->addWidget(config_stack_);

  panel_blank_ = new Panel();
  config_stack_->addWidget(panel_blank_);

  panel_mxhd_ = new MxHdPanel();
  config_stack_->addWidget(panel_mxhd_);

  panel_riff_ = new RIFFPanel();
  config_stack_->addWidget(panel_riff_);

  panel_mxch_ = new MxChPanel();
  config_stack_->addWidget(panel_mxch_);

  panel_mxof_ = new MxOfPanel();
  config_stack_->addWidget(panel_mxof_);

  panel_mxob_ = new MxObPanel();
  config_stack_->addWidget(panel_mxob_);

  auto btn_layout = new QHBoxLayout();
  layout->addLayout(btn_layout);

  btn_layout->addStretch();

  auto accept_btn = new QPushButton(mode == Import ? tr("De-Weave") : tr("Weave"));
  accept_btn->setDefault(true);
  connect(accept_btn, &QPushButton::clicked, this, &SIViewDialog::accept);
  btn_layout->addWidget(accept_btn);

  auto reject_btn = new QPushButton(tr("Cancel"));
  connect(reject_btn, &QPushButton::clicked, this, &SIViewDialog::reject);
  btn_layout->addWidget(reject_btn);

  btn_layout->addStretch();
}

void SIViewDialog::SetPanel(Panel *panel, si::Chunk *chunk)
{
  auto current = static_cast<Panel*>(config_stack_->currentWidget());
  current->SetData(nullptr);

  config_stack_->setCurrentWidget(panel);
  panel->SetData(chunk);
  last_set_data_ = chunk;
}

void SIViewDialog::SelectionChanged(const QModelIndex &index)
{
  Panel *p = panel_blank_;
  Chunk *c = static_cast<Chunk*>(index.internalPointer());

  if (c) {
    switch (c->type()) {
    case Chunk::TYPE_MxHd:
      p = panel_mxhd_;
      break;
    case Chunk::TYPE_RIFF:
    case Chunk::TYPE_LIST:
      p = panel_riff_;
      break;
    case Chunk::TYPE_MxCh:
      p = panel_mxch_;
      break;
    case Chunk::TYPE_MxOf:
      p = panel_mxof_;
      break;
    case Chunk::TYPE_MxOb:
      p = panel_mxob_;
      break;
    case Chunk::TYPE_MxSt:
    case Chunk::TYPE_pad_:
      break;
    }
  }

  if (p != config_stack_->currentWidget() || c != last_set_data_) {
    SetPanel(p, c);
  }
}
