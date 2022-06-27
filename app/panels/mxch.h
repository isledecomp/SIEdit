#ifndef MXCHPANEL_H
#define MXCHPANEL_H

#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>

#include "panel.h"

class MxChPanel : public Panel
{
  Q_OBJECT
public:
  explicit MxChPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(si::Chunk *chunk) override;
  virtual void OnClosingData(si::Chunk *chunk) override;

private:
  QLineEdit *flag_edit_;
  QSpinBox *obj_edit_;
  QSpinBox *ms_offset_edit_;
  QSpinBox *data_sz_edit_;
  QTextEdit *data_edit_;

  QVector<QCheckBox*> flag_checkboxes_;

private slots:
  void FlagCheckBoxClicked(bool e);

};

#endif // MXCHPANEL_H
