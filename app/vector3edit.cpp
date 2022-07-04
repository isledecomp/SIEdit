#include "vector3edit.h"

#include <QHBoxLayout>
#include <QLabel>

Vector3Edit::Vector3Edit(QWidget *parent) :
  QWidget{parent}
{
  auto layout = new QHBoxLayout(this);

  layout->addWidget(new QLabel("X"));

  x_edit_ = new QDoubleSpinBox();
  x_edit_->setMinimum(DBL_MIN);
  x_edit_->setMaximum(DBL_MAX);
  layout->addWidget(x_edit_);

  layout->addStretch();

  layout->addWidget(new QLabel("Y"));

  y_edit_ = new QDoubleSpinBox();
  y_edit_->setMinimum(DBL_MIN);
  y_edit_->setMaximum(DBL_MAX);
  layout->addWidget(y_edit_);

  layout->addStretch();

  layout->addWidget(new QLabel("Z"));

  z_edit_ = new QDoubleSpinBox();
  z_edit_->setMinimum(DBL_MIN);
  z_edit_->setMaximum(DBL_MAX);
  layout->addWidget(z_edit_);
}

si::Vector3 Vector3Edit::GetValue() const
{
  return si::Vector3(x_edit_->value(), y_edit_->value(), z_edit_->value());
}

void Vector3Edit::SetValue(const si::Vector3 &xyz)
{
  x_edit_->setValue(xyz.x);
  y_edit_->setValue(xyz.y);
  z_edit_->setValue(xyz.z);
}
