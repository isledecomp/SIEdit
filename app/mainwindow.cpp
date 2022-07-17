#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>

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
  tree_->setModel(&model_);
  tree_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tree_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::SelectionChanged);
  connect(tree_, &QTreeView::customContextMenuRequested, this, &MainWindow::ShowContextMenu);
  splitter->addWidget(tree_);

  auto config_area = new QWidget();
  splitter->addWidget(config_area);

  auto config_layout = new QVBoxLayout(config_area);

  action_grp_ = new QGroupBox();
  config_layout->addWidget(action_grp_);

  auto action_layout = new QHBoxLayout(action_grp_);

  action_layout->addStretch();

  auto extract_btn = new QPushButton(tr("Extract"));
  action_layout->addWidget(extract_btn);
  connect(extract_btn, &QPushButton::clicked, this, &MainWindow::ExtractClicked);

  auto replace_btn = new QPushButton(tr("Replace"));
  action_layout->addWidget(replace_btn);

  action_layout->addStretch();

  config_stack_ = new QStackedWidget();
  config_layout->addWidget(config_stack_);

  panel_blank_ = new Panel();
  config_stack_->addWidget(panel_blank_);

  panel_wav_ = new WavPanel();
  config_stack_->addWidget(panel_wav_);

  panel_bmp_ = new BitmapPanel();
  config_stack_->addWidget(panel_bmp_);

  InitializeMenuBar();

  splitter->setSizes({99999, 99999});

  setWindowTitle(tr("SI Editor"));
}

void MainWindow::OpenFilename(const QString &s)
{
  Chunk si;
  if (si.Read(s.toStdString())) {
    SIViewDialog d(SIViewDialog::Import, &si, this);
    if (d.exec() == QDialog::Accepted) {
      model_.SetCore(nullptr);
      interleaf_.Parse(&si);
      model_.SetCore(&interleaf_);
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
  connect(export_action, &QAction::triggered, this, &MainWindow::ExportFile);

  file_menu->addSeparator();

  auto exit_action = file_menu->addAction(tr("E&xit"));
  connect(exit_action, &QAction::triggered, this, &MainWindow::close);

  setMenuBar(menubar);
}

void MainWindow::SetPanel(Panel *panel, si::Object *chunk)
{
  auto current = static_cast<Panel*>(config_stack_->currentWidget());
  current->SetData(nullptr);

  config_stack_->setCurrentWidget(panel);
  panel->SetData(chunk);
  last_set_data_ = chunk;

  action_grp_->setEnabled(chunk);
}

void MainWindow::ExtractObject(si::Object *obj)
{
  QString filename = QString::fromStdString(obj->filename());
  if (filename.isEmpty()) {
    filename = QString::fromStdString(obj->name());
    filename.append(QStringLiteral(".bin"));
  } else {
    // Strip off directory
    int index = filename.lastIndexOf('\\');
    if (index != -1) {
      filename = filename.mid(index+1);
    }
  }

  QString s = QFileDialog::getSaveFileName(this, tr("Export Object"), filename);
  if (!s.isEmpty()) {
    QFile f(s);
    if (f.open(QFile::WriteOnly)) {
      bytearray b = obj->GetNormalizedData();
      f.write(b.data(), b.size());
      f.close();
    } else {
      QMessageBox::critical(this, QString(), tr("Failed to write to file \"%1\".").arg(s));
    }
  }
}

void MainWindow::OpenFile()
{
  QString s = QFileDialog::getOpenFileName(this, QString(), QString(), tr("Interleaf Files (*.si)"));
  if (!s.isEmpty()) {
    OpenFilename(s);
  }
}

void MainWindow::ExportFile()
{
  Chunk *c = interleaf_.Export();
  SIViewDialog d(SIViewDialog::Export, c, this);
  if (d.exec() == QDialog::Accepted) {
    QString s = QFileDialog::getSaveFileName(this);
    if (!s.isEmpty()) {
      if (!c->Write(s.toStdString())) {
        QMessageBox::critical(this, QString(), tr("Failed to write SI file."));
      }
    }
  }
  delete c;
}

void MainWindow::SelectionChanged(const QModelIndex &index)
{
  Panel *p = panel_blank_;
  Object *c = dynamic_cast<Object*>(static_cast<Core*>(index.internalPointer()));

  if (c) {
    switch (c->filetype()) {
    case MxOb::WAV:
      p = panel_wav_;
      break;
    case MxOb::STL:
      p = panel_bmp_;
      break;
    case MxOb::SMK:
    case MxOb::FLC:
    case MxOb::OBJ:
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
  auto selected = tree_->selectionModel()->selectedRows();
  if (selected.empty()) {
    return;
  }

  for (const QModelIndex &i : selected) {
    if (Object *obj = dynamic_cast<Object*>(static_cast<Core*>(i.internalPointer()))) {
      ExtractObject(obj);
    }
  }
}

void MainWindow::ExtractClicked()
{
  ExtractObject(last_set_data_);
}
