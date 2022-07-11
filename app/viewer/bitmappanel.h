#ifndef BITMAPPANEL_H
#define BITMAPPANEL_H

#include <panel.h>
#include <QLabel>

class BitmapPanel : public Panel
{
  Q_OBJECT
public:
  BitmapPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(void *data) override;
  //virtual void OnClosingData(void *data) override;

private:
  QLabel *img_lbl_;

  QLabel *desc_lbl_;

};

#endif // BITMAPPANEL_H
