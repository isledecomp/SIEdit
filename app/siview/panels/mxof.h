#ifndef MXOFPANEL_H
#define MXOFPANEL_H

#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>

#include "panel.h"

class MxOfPanel : public Panel
{
  Q_OBJECT
public:
  explicit MxOfPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  QSpinBox *obj_count_edit_;

  QListWidget *list_;

};

#endif // MXOFPANEL_H
