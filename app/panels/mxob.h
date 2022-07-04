#ifndef MXOBPANEL_H
#define MXOBPANEL_H

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>

#include "panel.h"
#include "vector3edit.h"

class MxObPanel : public Panel
{
  Q_OBJECT
public:
  explicit MxObPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(si::Chunk *chunk) override;
  virtual void OnClosingData(si::Chunk *chunk) override;

private:
  QComboBox *type_combo_;
  QLineEdit *name_edit_;
  QLineEdit *presenter_edit_;
  QLineEdit *filename_edit_;
  QSpinBox *obj_id_edit_;

  Vector3Edit *pos_edit_;
  Vector3Edit *dir_edit_;
  Vector3Edit *up_edit_;

};

#endif // MXOBPANEL_H
