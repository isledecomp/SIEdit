#ifndef MXHDPANEL_H
#define MXHDPANEL_H

#include <data/mxhd.h>
#include <QLineEdit>
#include <QSpinBox>

#include "panel.h"

class MxHdPanel : public Panel
{
  Q_OBJECT
public:
  explicit MxHdPanel(QWidget *parent = nullptr);

signals:

protected:
  virtual void OnOpeningData(Data *data) override;
  virtual void OnClosingData(Data *data) override;

private:
  QSpinBox *major_version_edit_;
  QSpinBox *minor_version_edit_;
  QSpinBox *buffer_alignment_edit_;
  QSpinBox *buffer_count_edit_;



};

#endif // MXHDPANEL_H
