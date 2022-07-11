#include "mxch.h"

#include <chunk.h>
#include <QFontDatabase>
#include <QGroupBox>
#include <QLabel>

using namespace si;

MxChPanel::MxChPanel(QWidget *parent) :
  Panel(parent)
{
  int row = 0;

  auto flag_group = new QGroupBox(tr("Flags"));
  layout()->addWidget(flag_group, row, 0, 1, 2);

  {
    auto flag_layout = new QGridLayout(flag_group);

    int flag_row = 0;

    flag_layout->addWidget(new QLabel(tr("Value")), flag_row, 0);

    flag_edit_ = new QLineEdit();
    flag_layout->addWidget(flag_edit_, flag_row, 1);

    flag_row++;

    auto split_chunk = new QCheckBox(tr("Split Chunk"));
    split_chunk->setProperty("flag", MxCh::FLAG_SPLIT);
    connect(split_chunk, &QCheckBox::clicked, this, &MxChPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(split_chunk);
    flag_layout->addWidget(split_chunk, flag_row, 0, 1, 2);

    flag_row++;

    auto end_chunk = new QCheckBox(tr("End Chunk"));
    end_chunk->setProperty("flag", MxCh::FLAG_END);
    connect(end_chunk, &QCheckBox::clicked, this, &MxChPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(end_chunk);
    flag_layout->addWidget(end_chunk, flag_row, 0, 1, 2);

    flag_row++;
  }

  row++;

  layout()->addWidget(new QLabel(tr("Object ID")), row, 0);

  obj_edit_ = new QSpinBox();
  obj_edit_->setMinimum(0);
  obj_edit_->setMaximum(INT_MAX);
  layout()->addWidget(obj_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Millisecond Offset")), row, 0);

  ms_offset_edit_ = new QSpinBox();
  ms_offset_edit_->setMinimum(0);
  ms_offset_edit_->setMaximum(INT_MAX);
  layout()->addWidget(ms_offset_edit_, row, 1);

  row++;

  layout()->addWidget(new QLabel(tr("Data Size")), row, 0);

  data_sz_edit_ = new QSpinBox();
  data_sz_edit_->setMinimum(0);
  data_sz_edit_->setMaximum(INT_MAX);
  layout()->addWidget(data_sz_edit_, row, 1);

  row++;

  show_data_btn_ = new QPushButton(tr("Show Data"));
  connect(show_data_btn_, &QPushButton::clicked, this, &MxChPanel::ShowDataField);
  layout()->addWidget(show_data_btn_, row, 0, 1, 2);

  data_edit_ = new QTextEdit();
  data_edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  layout()->addWidget(data_edit_, row, 0, 1, 2);
  data_edit_->hide();

  FinishLayout();
}

void MxChPanel::OnOpeningData(void *d)
{
  Chunk *chunk = static_cast<Chunk*>(d);

  flag_edit_->setText(QString::number(chunk->data("Flags"), 16));
  obj_edit_->setValue(chunk->data("Object"));
  ms_offset_edit_->setValue(chunk->data("Time"));
  data_sz_edit_->setValue(chunk->data("DataSize"));

  for (QCheckBox *cb : qAsConst(flag_checkboxes_)) {
    cb->setChecked(cb->property("flag").toUInt() & chunk->data("Flags"));
  }
}

void MxChPanel::OnClosingData(void *data)
{
  /*bool ok;
  u16 flags = flag_edit_->text().toUShort(&ok, 16);
  if (ok) {
    chunk->data("Flags") = flags;
  }

  chunk->data("Object") = u32(obj_edit_->value());
  chunk->data("Time") = u32(ms_offset_edit_->value());
  chunk->data("DataSize") = u32(data_sz_edit_->value());*/

  show_data_btn_->show();
  data_edit_->clear();
  data_edit_->hide();
}

void MxChPanel::FlagCheckBoxClicked(bool e)
{
  uint16_t flag = sender()->property("flag").toUInt();
  bool ok;
  uint16_t current = flag_edit_->text().toUShort(&ok, 16);
  if (ok) {
    if (e) {
      current |= flag;
    } else {
      current &= ~flag;
    }

    flag_edit_->setText(QString::number(current, 16));
  }
}

void MxChPanel::ShowDataField()
{
  show_data_btn_->hide();
  data_edit_->show();

  Chunk *chunk = static_cast<Chunk*>(GetData());

  const Data &data = chunk->data("Data");
  QByteArray ba(data.data(), int(data.size()));
  data_edit_->setPlainText(ba.toHex());
}
