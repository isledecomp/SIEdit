#include "mainwindow.h"

#include <iostream>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>

#include "siview/siview.h"

using namespace si;

const QString MainWindow::kFileFilter = tr("Interleaf Files (*.si)");

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
  connect(replace_btn, &QPushButton::clicked, this, &MainWindow::ReplaceClicked);

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
  model_.SetCore(nullptr);

  if (OpenInterleafFileInternal(this, &interleaf_, s)) {
    model_.SetCore(&interleaf_);
  }
}

void MainWindow::InitializeMenuBar()
{
  auto menubar = new QMenuBar();

  auto file_menu = menubar->addMenu(tr("&File"));

  file_menu->addAction(tr("&New"), this, &MainWindow::NewFile, tr("Ctrl+N"));

  file_menu->addAction(tr("&Open"), this, &MainWindow::OpenFile, tr("Ctrl+O"));

  file_menu->addAction(tr("&Save"), this, &MainWindow::SaveFile, tr("Ctrl+S"));

  file_menu->addAction(tr("Save &As"), this, &MainWindow::SaveFileAs, tr("Ctrl+Shift+S"));

  file_menu->addSeparator();

  file_menu->addAction(tr("&View SI File"), this, &MainWindow::ViewSIFile);

  file_menu->addSeparator();

  file_menu->addAction(tr("E&xit"), this, &MainWindow::close);

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
    if (!obj->ExtractToFile(
#ifdef Q_OS_WINDOWS
          s.toStdWString().c_str()
#else
          s.toUtf8()
#endif
          )) {
      QMessageBox::critical(this, QString(), tr("Failed to write to file \"%1\".").arg(s));
    }
  }
}

void MainWindow::ReplaceObject(si::Object *obj)
{
  QString s = QFileDialog::getOpenFileName(this, tr("Replace Object"));
  if (!s.isEmpty()) {
    if (!obj->ReplaceWithFile(
#ifdef Q_OS_WINDOWS
          s.toStdWString().c_str()
#else
          s.toUtf8()
#endif
        )) {
      QMessageBox::critical(this, QString(), tr("Failed to open to file \"%1\".").arg(s));
    }
  }
}

bool MainWindow::OpenInterleafFileInternal(QWidget *parent, si::Interleaf *interleaf, const QString &s)
{
  Interleaf::Error r = interleaf->Read(
#ifdef Q_OS_WINDOWS
    s.toStdWString().c_str()
#else
    s.toUtf8()
#endif
  );

  if (r == Interleaf::ERROR_SUCCESS) {
    return true;
  } else {
    QMessageBox::critical(parent, QString(), tr("Failed to load Interleaf file: %1").arg(r));
    return false;
  }
}

QString MainWindow::GetOpenFileName()
{
  return QFileDialog::getOpenFileName(this, QString(), QString(), kFileFilter);
}

void MainWindow::NewFile()
{
  model_.SetCore(nullptr);
  interleaf_.Clear();
  model_.SetCore(&interleaf_);
}

void MainWindow::OpenFile()
{
  QString s = GetOpenFileName();
  if (!s.isEmpty()) {
    OpenFilename(s);
  }
}

bool MainWindow::SaveFile()
{
  if (current_filename_.isEmpty()) {
    return SaveFileAs();
  } else {
    Interleaf::Error r = interleaf_.Write(
#ifdef Q_OS_WINDOWS
      current_filename_.toStdWString().c_str()
#else
      current_filename_.toUtf8()
#endif
    );

    if (r == Interleaf::ERROR_SUCCESS) {
      return true;
    } else {
      QMessageBox::critical(this, QString(), tr("Failed to write SI file: %1").arg(r));
      return false;
    }
  }
}

bool MainWindow::SaveFileAs()
{
  current_filename_ = QFileDialog::getSaveFileName(this, QString(), QString(), kFileFilter);
  if (!current_filename_.isEmpty()) {
    return SaveFile();
  }

  return false;
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

void MainWindow::ReplaceClicked()
{
  ReplaceObject(last_set_data_);
}

void MainWindow::ViewSIFile()
{
  QString s = GetOpenFileName();
  if (!s.isEmpty()) {
    std::unique_ptr<Interleaf> temp = std::make_unique<Interleaf>();
    if (OpenInterleafFileInternal(this, temp.get(), s)) {
      SIViewDialog *v = new SIViewDialog(temp->GetInformation(), this);
      v->temp = std::move(temp);
      v->setAttribute(Qt::WA_DeleteOnClose);
      v->show();
    }
  }
}
