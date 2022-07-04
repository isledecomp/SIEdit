#include "mxob.h"

#include <sitypes.h>
#include <QGroupBox>
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

  row++;

  layout()->addWidget(new QLabel(tr("Duration")), row, 0);

  duration_edit_ = new QSpinBox();
  duration_edit_->setMinimum(0);
  duration_edit_->setMaximum(INT_MAX);
  layout()->addWidget(duration_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Loops")), row, 0);

  loops_edit_ = new QSpinBox();
  loops_edit_->setMinimum(0);
  loops_edit_->setMaximum(INT_MAX);
  layout()->addWidget(loops_edit_, row, 1);
  
  row++;

  auto pos_grp = new QGroupBox(tr("Position"));
  auto pos_lyt = new QVBoxLayout(pos_grp);
  pos_lyt->setMargin(0);
  pos_edit_ = new Vector3Edit();
  pos_lyt->addWidget(pos_edit_);
  layout()->addWidget(pos_grp, row, 0, 1, 2);

  row++;

  auto dir_grp = new QGroupBox(tr("Direction"));
  auto dir_lyt = new QVBoxLayout(dir_grp);
  dir_lyt->setMargin(0);
  dir_edit_ = new Vector3Edit();
  dir_lyt->addWidget(dir_edit_);
  layout()->addWidget(dir_grp, row, 0, 1, 2);

  row++;

  auto up_grp = new QGroupBox(tr("Up"));
  auto up_lyt = new QVBoxLayout(up_grp);
  up_lyt->setMargin(0);
  up_edit_ = new Vector3Edit();
  up_lyt->addWidget(up_edit_);
  layout()->addWidget(up_grp, row, 0, 1, 2);

  FinishLayout();
}

void MxObPanel::OnOpeningData(si::Chunk *chunk)
{
  type_combo_->setCurrentIndex(chunk->data("Type"));
  name_edit_->setText(QString(chunk->data("Name")));
  filename_edit_->setText(QString(chunk->data("FileName")));
  presenter_edit_->setText(QString(chunk->data("Presenter")));
  obj_id_edit_->setValue(chunk->data("ID"));

  duration_edit_->setValue(chunk->data("Duration"));
  loops_edit_->setValue(chunk->data("Loops"));

  pos_edit_->SetValue(chunk->data("Position"));
  dir_edit_->SetValue(chunk->data("Direction"));
  up_edit_->SetValue(chunk->data("Up"));
}

void MxObPanel::OnClosingData(si::Chunk *chunk)
{
  chunk->data("Type") = type_combo_->currentIndex();

  chunk->data("Duration") = duration_edit_->value();
  chunk->data("Loops") = loops_edit_->value();

  chunk->data("Position") = pos_edit_->GetValue();
  chunk->data("Direction") = dir_edit_->GetValue();
  chunk->data("Up") = up_edit_->GetValue();
}
