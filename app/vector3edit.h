#ifndef VECTOR3EDIT_H
#define VECTOR3EDIT_H

#include <QDoubleSpinBox>
#include <QWidget>

#include <types.h>

class Vector3Edit : public QWidget
{
  Q_OBJECT
public:
  explicit Vector3Edit(QWidget *parent = nullptr);

  si::Vector3 GetValue() const;
  void SetValue(const si::Vector3 &xyz);

private:
  QDoubleSpinBox *x_edit_;
  QDoubleSpinBox *y_edit_;
  QDoubleSpinBox *z_edit_;

};

#endif // VECTOR3EDIT_H
