#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QSplitter>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow{parent},
  last_set_data_(nullptr)
{
  auto splitter = new QSplitter();
  splitter->setChildrenCollapsible(false);
  this->setCentralWidget(splitter);

  model_.SetChunk(&chunk_);

  auto tree = new QTreeView();
  tree->setModel(&model_);
  splitter->addWidget(tree);
  connect(tree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::SelectionChanged);

  config_stack_ = new QStackedWidget();
  splitter->addWidget(config_stack_);

  panel_blank_ = new Panel();
  config_stack_->addWidget(panel_blank_);

  panel_mxhd_ = new MxHdPanel();
  config_stack_->addWidget(panel_mxhd_);

  InitializeMenuBar();

  splitter->setSizes({99999, 99999});
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

void MainWindow::SetPanel(Panel *panel, Data *data)
{
  auto current = static_cast<Panel*>(config_stack_->currentWidget());
  current->SetData(nullptr);

  config_stack_->setCurrentWidget(panel);
  panel->SetData(data);
  last_set_data_ = data;
}

void MainWindow::OpenFile()
{
  QString s = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Interleaf Files (*.si)"));
  if (!s.isEmpty()) {
    model_.SetChunk(nullptr);
    SetPanel(panel_blank_, nullptr);
    chunk_.Read(s.toStdString());
    model_.SetChunk(&chunk_);
  }
}

void MainWindow::SelectionChanged(const QModelIndex &index)
{
  Panel *panel_to_set = panel_blank_;
  Data *data = nullptr;

  if (Chunk *c = static_cast<Chunk*>(index.internalPointer())) {
    data = c->data();

    switch (c->type()) {
    case Chunk::MxHd:
      panel_to_set = panel_mxhd_;
      break;
    case Chunk::RIFF:
    case Chunk::LIST:
    case Chunk::MxSt:
    case Chunk::pad_:
      break;
    }
  }

  if (panel_to_set != config_stack_->currentWidget() || data != last_set_data_) {
    SetPanel(panel_to_set, data);
  }
}
