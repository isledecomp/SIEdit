#ifndef INFOPANEL_H
#define INFOPANEL_H

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>

#include "panel.h"

class InfoPanel : public Panel
{
  Q_OBJECT
public:
  InfoPanel(QWidget *parent = nullptr);

protected:
  virtual void OnOpeningData(void *data) override;
  virtual void OnClosingData(void *data) override;

private:
  QLabel *m_Lbl;

  QPushButton *m_ShowDataBtn;

  QPlainTextEdit *m_DataView;

private slots:
  void ShowData();

};

#endif // INFOPANEL_H
