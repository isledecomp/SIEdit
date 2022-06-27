#include "panel.h"

Panel::Panel(QWidget *parent) :
  QWidget{parent},
  chunk_(nullptr)
{
  outer_layout_ = new QVBoxLayout(this);

  layout_ = new QGridLayout();
  outer_layout_->addLayout(layout_);
}

void Panel::SetData(si::Chunk *chunk)
{
  if (chunk_) {
    OnClosingData(chunk_);
  }

  chunk_ = chunk;

  if (chunk_) {
    OnOpeningData(chunk_);
  }
}

void Panel::FinishLayout()
{
  outer_layout_->addStretch();
}
