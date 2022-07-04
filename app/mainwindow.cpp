#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QSplitter>

using namespace si;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow{parent},
  last_set_data_(nullptr)
{
  auto splitter = new QSplitter();
  splitter->setChildrenCollapsible(false);
  this->setCentralWidget(splitter);

  auto tree_tab = new QTabWidget();
  splitter->addWidget(tree_tab);

  /*auto simple_tree = new QTreeView();
  simple_tree->setModel(&object_model_);
  tree_tab->addTab(simple_tree, tr("Simple"));
  connect(simple_tree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::SelectionChanged);*/

  lowlevel_tree_ = new QTreeView();
  lowlevel_tree_->setModel(&chunk_model_);
  lowlevel_tree_->setContextMenuPolicy(Qt::CustomContextMenu);
  tree_tab->addTab(lowlevel_tree_, tr("Advanced"));
  connect(lowlevel_tree_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::SelectionChanged);
  connect(lowlevel_tree_, &QTreeView::customContextMenuRequested, this, &MainWindow::ShowContextMenu);

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

  InitializeMenuBar();

  splitter->setSizes({99999, 99999});
}

void MainWindow::OpenFilename(const QString &s)
{
  object_model_.SetChunk(nullptr);
  chunk_model_.SetChunk(nullptr);
  SetPanel(panel_blank_, nullptr);
  chunk_.Read(s.toStdString());
  object_model_.SetChunk(&chunk_);
  chunk_model_.SetChunk(&chunk_);
}

void MainWindow::InitializeMenuBar()
{
  auto menubar = new QMenuBar();

  auto file_menu = menubar->addMenu(tr("&File"));

  auto open_action = file_menu->addAction(tr("&Open"), this, &MainWindow::OpenFile, tr("Ctrl+O"));

  auto save_action = file_menu->addAction(tr("&Save"));
  auto save_as_action = file_menu->addAction(tr("Save &As"));

  file_menu->addSeparator();

  auto exit_action = file_menu->addAction(tr("E&xit"));
  connect(exit_action, &QAction::triggered, this, &MainWindow::close);

  setMenuBar(menubar);
}

void MainWindow::SetPanel(Panel *panel, si::Chunk *chunk)
{
  auto current = static_cast<Panel*>(config_stack_->currentWidget());
  current->SetData(nullptr);

  config_stack_->setCurrentWidget(panel);
  panel->SetData(chunk);
  last_set_data_ = chunk;
}

void MainWindow::OpenFile()
{
  QString s = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Interleaf Files (*.si)"));
  if (!s.isEmpty()) {
    OpenFilename(s);
  }
}

void MainWindow::SelectionChanged(const QModelIndex &index)
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

void MainWindow::ShowContextMenu(const QPoint &p)
{
  QMenu menu(this);

  QAction *extract_action = menu.addAction(tr("E&xtract"));
  connect(extract_action, &QAction::triggered, this, &MainWindow::ExtractSelectedItems);

  menu.exec(static_cast<QWidget*>(sender())->mapToGlobal(p));
}

void MainWindow::ExtractSelectedItems()
{
  auto selected = lowlevel_tree_->selectionModel()->selectedRows();
  if (selected.empty()) {
    return;
  }

  for (const QModelIndex &i : selected) {
    if (Chunk *chunk = static_cast<Chunk*>(i.internalPointer())) {
      QString filename(chunk->data("FileName"));
      if (filename.isEmpty()) {
        filename = QString(chunk->data("Name"));
        filename.append(QStringLiteral(".bin"));
      }
      if (filename.isEmpty()) {
        filename = QStringLiteral("%1_%2.bin").arg(QString::fromLatin1((const char *) &chunk->id(), sizeof(u32)),
                                                   QString::number(chunk->offset(), 16));
      }

      QString s = QFileDialog::getSaveFileName(this, tr("Export Object"), filename);
      if (!s.isEmpty()) {
        //chunk->Export()
      }
    }
  }
}
