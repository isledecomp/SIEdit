#include "panel.h"

Panel::Panel(QWidget *parent) :
  QWidget{parent},
  data_(nullptr)
{
  outer_layout_ = new QVBoxLayout(this);
  outer_layout_->setContentsMargins(0, 0, 0, 0);

  layout_ = new QGridLayout();
  outer_layout_->addLayout(layout_);
}

void Panel::SetData(void *data)
{
  if (data_) {
    OnClosingData(data_);
  }

  data_ = data;

  if (data_) {
    OnOpeningData(data_);
  }
}

void Panel::ResetData()
{
  SetData(data_);
}

void Panel::FinishLayout()
{
  outer_layout_->addStretch();
}
