#ifndef PANEL_H
#define PANEL_H

#include <QGridLayout>
#include <QWidget>

class Panel : public QWidget
{
  Q_OBJECT
public:
  explicit Panel(QWidget *parent = nullptr);

  void *GetData() const { return data_; }
  void SetData(void *data);

signals:

protected:
  virtual void OnOpeningData(void *data){}
  virtual void OnClosingData(void *data){}

  QGridLayout *layout() const { return layout_; }

  void FinishLayout();

private:
  void *data_;

  QVBoxLayout *outer_layout_;
  QGridLayout *layout_;

};

#endif // PANEL_H
