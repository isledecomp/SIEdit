#include "panel.h"

Panel::Panel(QWidget *parent)
  : QWidget{parent},
    data_(nullptr)
{

}

void Panel::SetData(Data *data)
{
  if (data_) {
    OnClosingData(data_);
  }

  data_ = data;

  if (data_) {
    OnOpeningData(data_);
  }
}
