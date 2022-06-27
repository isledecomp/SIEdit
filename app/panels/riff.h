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
  virtual void OnOpeningData(si::Chunk *chunk) override;
  virtual void OnClosingData(si::Chunk *chunk) override;

private:
  QLineEdit *id_edit_;

};

#endif // RIFFPANEL_H
