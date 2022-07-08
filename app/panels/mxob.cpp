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

  auto flag_group = new QGroupBox(tr("Flags"));
  layout()->addWidget(flag_group, row, 0, 1, 2);

  row++;

  {
    auto flag_layout = new QGridLayout(flag_group);

    int flag_row = 0;

    flag_layout->addWidget(new QLabel(tr("Value")), flag_row, 0);

    flag_edit_ = new QLineEdit();
    flag_layout->addWidget(flag_edit_, flag_row, 1);

    flag_row++;

    auto loop_cache = new QCheckBox(tr("Loop from Cache"));
    loop_cache->setProperty("flag", 0x01);
    connect(loop_cache, &QCheckBox::clicked, this, &MxObPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(loop_cache);
    flag_layout->addWidget(loop_cache, flag_row, 0, 1, 2);

    flag_row++;

    auto no_loop = new QCheckBox(tr("No Loop"));
    no_loop->setProperty("flag", 0x02);
    connect(no_loop, &QCheckBox::clicked, this, &MxObPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(no_loop);
    flag_layout->addWidget(no_loop, flag_row, 0, 1, 2);

    flag_row++;

    auto loop_stream = new QCheckBox(tr("Loop from Stream"));
    loop_stream->setProperty("flag", 0x04);
    connect(loop_stream, &QCheckBox::clicked, this, &MxObPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(loop_stream);
    flag_layout->addWidget(loop_stream, flag_row, 0, 1, 2);

    flag_row++;

    auto transparent = new QCheckBox(tr("Transparent"));
    transparent->setProperty("flag", 0x08);
    connect(transparent, &QCheckBox::clicked, this, &MxObPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(transparent);
    flag_layout->addWidget(transparent, flag_row, 0, 1, 2);

    flag_row++;

    auto unknown = new QCheckBox(tr("Unknown"));
    unknown->setProperty("flag", 0x20);
    connect(unknown, &QCheckBox::clicked, this, &MxObPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(unknown);
    flag_layout->addWidget(unknown, flag_row, 0, 1, 2);

    flag_row++;

  }

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

  flag_edit_->setText(QString::number(chunk->data("Flags"), 16));

  for (QCheckBox* cb : qAsConst(flag_checkboxes_)) {
    cb->setChecked(cb->property("flag").toUInt() & chunk->data("Flags"));
  }

  duration_edit_->setValue(chunk->data("Duration"));
  loops_edit_->setValue(chunk->data("Loops"));

  pos_edit_->SetValue(chunk->data("Position"));
  dir_edit_->SetValue(chunk->data("Direction"));
  up_edit_->SetValue(chunk->data("Up"));
}

void MxObPanel::OnClosingData(si::Chunk *chunk)
{
  chunk->data("Type") = type_combo_->currentIndex();

  bool ok;
  u32 flags = flag_edit_->text().toUInt(&ok, 16);
  if (ok) {
    chunk->data("Flags") = flags;
  }

  chunk->data("Duration") = duration_edit_->value();
  chunk->data("Loops") = loops_edit_->value();

  chunk->data("Position") = pos_edit_->GetValue();
  chunk->data("Direction") = dir_edit_->GetValue();
  chunk->data("Up") = up_edit_->GetValue();
}

void MxObPanel::FlagCheckBoxClicked(bool e)
{
  u32 flag = sender()->property("flag").toUInt();
  bool ok;
  u32 current = flag_edit_->text().toUInt(&ok, 16);
  if (ok) {
    if (e) {
      current |= flag;
    }
    else {
      current &= ~flag;
    }

    flag_edit_->setText(QString::number(current, 16));
  }
}