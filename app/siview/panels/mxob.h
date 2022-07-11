#ifndef MXOBPANEL_H
#define MXOBPANEL_H

#include <QCheckBox>
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
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  QComboBox *type_combo_;
  QLineEdit *name_edit_;
  QLineEdit *presenter_edit_;
  QLineEdit *filename_edit_;
  QSpinBox *obj_id_edit_;

  QLineEdit* flag_edit_;

  QSpinBox *duration_edit_;
  QSpinBox *loops_edit_;

  Vector3Edit *pos_edit_;
  Vector3Edit *dir_edit_;
  Vector3Edit *up_edit_;

  QVector<QCheckBox*> flag_checkboxes_;

private slots:
  void FlagCheckBoxClicked(bool e);

};

#endif // MXOBPANEL_H
