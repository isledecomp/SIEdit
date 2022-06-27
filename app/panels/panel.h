#ifndef PANEL_H
#define PANEL_H

#include <chunk.h>
#include <QGridLayout>
#include <QWidget>

class Panel : public QWidget
{
  Q_OBJECT
public:
  explicit Panel(QWidget *parent = nullptr);

  void SetData(si::Chunk *chunk);

signals:

protected:
  virtual void OnOpeningData(si::Chunk *chunk){}
  virtual void OnClosingData(si::Chunk *chunk){}

  QGridLayout *layout() const { return layout_; }

  void FinishLayout();

private:
  si::Chunk *chunk_;

  QVBoxLayout *outer_layout_;
  QGridLayout *layout_;

};

#endif // PANEL_H
