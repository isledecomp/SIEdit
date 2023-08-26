#include "infopanel.h"

#include <info.h>
#include <QDebug>
#include <QScrollArea>

InfoPanel::InfoPanel(QWidget *parent) :
  Panel(parent)
{
  int row = 0;

  QScrollArea *scrollArea = new QScrollArea(this);
//  scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  scrollArea->setWidgetResizable(true);
  layout()->addWidget(scrollArea, row, 0);

  m_Lbl = new QLabel();
  m_Lbl->setAlignment(Qt::AlignLeft);
  scrollArea->setWidget(m_Lbl);

  row++;

  m_ShowDataBtn = new QPushButton(tr("Show Data"));
  connect(m_ShowDataBtn, &QPushButton::clicked, this, &InfoPanel::ShowData);
  layout()->addWidget(m_ShowDataBtn, row, 0);
  m_ShowDataBtn->hide();

  row++;

  m_DataView = new QPlainTextEdit();
  m_DataView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  layout()->addWidget(m_DataView, row, 0);
  m_DataView->hide();

  FinishLayout();
}

void InfoPanel::OnOpeningData(void *data)
{
  auto info = static_cast<si::Info*>(data);
  m_Lbl->setText(QString::fromStdString(info->GetDescription()));

  if (!info->GetData().empty()) {
    m_ShowDataBtn->show();
  }
}

void InfoPanel::OnClosingData(void *data)
{
  m_Lbl->setText(QString());
  m_DataView->hide();
  m_DataView->clear();
  m_ShowDataBtn->hide();
}

void InfoPanel::ShowData()
{
  const si::bytearray &s = static_cast<si::Info*>(this->GetData())->GetData();

  m_ShowDataBtn->hide();
  m_DataView->setPlainText(QByteArray(s.data(), s.size()).toHex());
  m_DataView->show();
}
