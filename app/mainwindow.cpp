#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>

#include "interleaf.h"
#include "siview/siview.h"

using namespace si;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow{parent},
  last_set_data_(nullptr)
{
  auto splitter = new QSplitter();
  splitter->setChildrenCollapsible(false);
  this->setCentralWidget(splitter);

  tree_ = new QTreeView();
  //tree_->setModel(&chunk_model_);
  tree_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tree_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::SelectionChanged);
  connect(tree_, &QTreeView::customContextMenuRequested, this, &MainWindow::ShowContextMenu);
  splitter->addWidget(tree_);

  config_stack_ = new QStackedWidget();
  splitter->addWidget(config_stack_);

  panel_blank_ = new Panel();
  config_stack_->addWidget(panel_blank_);

  InitializeMenuBar();

  splitter->setSizes({99999, 99999});
}

void MainWindow::OpenFilename(const QString &s)
{
  Chunk si;
  if (si.Read(s.toStdString())) {
    SIViewDialog d(SIViewDialog::Import, &si, this);
    if (d.exec() == QDialog::Accepted) {
      Interleaf interleaf;
      interleaf.Parse(&si);
    }
  } else {
    QMessageBox::critical(this, QString(), tr("Failed to load Interleaf file."));
  }
}

void MainWindow::InitializeMenuBar()
{
  auto menubar = new QMenuBar();

  auto file_menu = menubar->addMenu(tr("&File"));

  auto new_action = file_menu->addAction(tr("&New"));

  auto open_action = file_menu->addAction(tr("&Open"), this, &MainWindow::OpenFile, tr("Ctrl+O"));

  auto save_action = file_menu->addAction(tr("&Save"));
  auto save_as_action = file_menu->addAction(tr("Save &As"));

  file_menu->addSeparator();

  auto export_action = file_menu->addAction(tr("&Export"));

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
    // HECK
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
  auto selected = tree_->selectionModel()->selectedRows();
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
