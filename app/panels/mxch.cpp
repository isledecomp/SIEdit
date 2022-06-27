#include "mxch.h"

#include <data/mxch.h>
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
    split_chunk->setProperty("flag", 0x10);
    connect(split_chunk, &QCheckBox::clicked, this, &MxChPanel::FlagCheckBoxClicked);
    flag_checkboxes_.append(split_chunk);
    flag_layout->addWidget(split_chunk, flag_row, 0, 1, 2);

    flag_row++;

    auto end_chunk = new QCheckBox(tr("End Chunk"));
    end_chunk->setProperty("flag", 0x2);
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

  data_edit_ = new QTextEdit();
  layout()->addWidget(data_edit_, row, 0, 1, 2);

  data_edit_->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

  FinishLayout();
}

void MxChPanel::OnOpeningData(Chunk *chunk)
{
  MxCh *mxch = chunk->data().cast<MxCh>();

  flag_edit_->setText(QString::number(mxch->wFlags, 16));
  obj_edit_->setValue(mxch->dwObjectParent);
  ms_offset_edit_->setValue(mxch->dwMillisecondOffset);
  data_sz_edit_->setValue(mxch->dwDataSize);

  for (QCheckBox *cb : qAsConst(flag_checkboxes_)) {
    cb->setChecked(cb->property("flag").toUInt() & mxch->wFlags);
  }

  QByteArray ba((const char*) chunk->exdata().data(), chunk->exdata().size());
  data_edit_->setPlainText(ba.toHex());
}

void MxChPanel::OnClosingData(Chunk *chunk)
{
  MxCh *mxch = chunk->data().cast<MxCh>();

  bool ok;
  u16 flags = flag_edit_->text().toUShort(&ok, 16);
  if (ok) {
    mxch->wFlags = flags;
  }

  mxch->dwObjectParent = obj_edit_->value();
  mxch->dwMillisecondOffset = ms_offset_edit_->value();
  mxch->dwDataSize = data_sz_edit_->value();
}

void MxChPanel::FlagCheckBoxClicked(bool e)
{
  u16 flag = sender()->property("flag").toUInt();
  bool ok;
  u16 current = flag_edit_->text().toUShort(&ok, 16);
  if (ok) {
    if (e) {
      current |= flag;
    } else {
      current &= ~flag;
    }

    flag_edit_->setText(QString::number(current, 16));
  }
}
