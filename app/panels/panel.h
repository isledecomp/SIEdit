#ifndef PANEL_H
#define PANEL_H

#include <data/data.h>
#include <QWidget>

class Panel : public QWidget
{
  Q_OBJECT
public:
  explicit Panel(QWidget *parent = nullptr);

  Data *GetData() const { return data_; }
  void SetData(Data *data);

signals:

protected:
  virtual void OnOpeningData(Data *data){}
  virtual void OnClosingData(Data *data){}

private:
  Data *data_;

};

#endif // PANEL_H
