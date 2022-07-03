#include "mxob.h"

#include <sitypes.h>
#include <QLabel>

using namespace si;

MxObPanel::MxObPanel(QWidget *parent) :
  Panel(parent)
{
  int row = 0;

  layout()->addWidget(new QLabel(tr("Type")), row, 0);

  type_combo_ = new QComboBox();
  for (int i=0; i<MxOb::TYPE_COUNT; i++) {
    type_combo_->addItem(MxOb::GetTypeName(MxOb::Type(i)));
  }
  layout()->addWidget(type_combo_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Name")), row, 0);

  name_edit_ = new QLineEdit();
  layout()->addWidget(name_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Filename")), row, 0);

  filename_edit_ = new QLineEdit();
  layout()->addWidget(filename_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Object ID")), row, 0);

  obj_id_edit_ = new QSpinBox();
  obj_id_edit_->setMinimum(0);
  obj_id_edit_->setMaximum(INT_MAX);
  layout()->addWidget(obj_id_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Presenter")), row, 0);

  presenter_edit_ = new QLineEdit();
  layout()->addWidget(presenter_edit_, row, 1);

  FinishLayout();
}

void MxObPanel::OnOpeningData(si::Chunk *chunk)
{
  type_combo_->setCurrentIndex(chunk->data("Type"));
  name_edit_->setText(QString(chunk->data("Name")));
  filename_edit_->setText(QString(chunk->data("FileName")));
  presenter_edit_->setText(QString(chunk->data("Presenter")));
  obj_id_edit_->setValue(chunk->data("ID"));
}

void MxObPanel::OnClosingData(si::Chunk *chunk)
{
  chunk->data("Type") = type_combo_->currentIndex();
}
