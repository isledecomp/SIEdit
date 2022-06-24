#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QSplitter>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow{parent}
{
  auto splitter = new QSplitter();
  this->setCentralWidget(splitter);

  model_.SetChunk(&chunk_);

  auto tree = new QTreeView();
  tree->setModel(&model_);
  splitter->addWidget(tree);

  InitializeMenuBar();
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

void MainWindow::OpenFile()
{
  QString s = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Interleaf Files (*.si)"));
  if (!s.isEmpty()) {
    chunk_.Read(s.toStdString());
    model_.SetChunk(&chunk_);
  }
}
