#ifndef RIFFPANEL_H
#define RIFFPANEL_H

#include <QLineEdit>

#include "panel.h"

class RIFFPanel : public Panel
{
  Q_OBJECT
public:
  explicit RIFFPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  QLineEdit *id_edit_;

};

#endif // RIFFPANEL_H
