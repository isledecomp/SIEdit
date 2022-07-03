#include "riff.h"

#include <QLabel>

using namespace si;

RIFFPanel::RIFFPanel(QWidget *parent) :
  Panel(parent)
{
  int row = 0;

  layout()->addWidget(new QLabel(tr("ID")), row, 0);

  id_edit_ = new QLineEdit();
  id_edit_->setMaxLength(sizeof(u32));
  layout()->addWidget(id_edit_, row, 1);

  FinishLayout();
}

void RIFFPanel::OnOpeningData(Chunk *chunk)
{
  QString s = QString::fromLatin1(chunk->data("Format").c_str(), sizeof(u32));
  id_edit_->setText(s);
}

void RIFFPanel::OnClosingData(Chunk *chunk)
{
  QByteArray d = id_edit_->text().toLatin1();

  const int target_sz = sizeof(u32);
  if (d.size() != target_sz) {
    int old_sz = d.size();
    d.resize(target_sz);
    for (int i=old_sz; i<target_sz; i++) {
      // Fill any empty space with spaces
      d[i] = ' ';
    }
  }

  chunk->data("Format") = *(u32*)d.data();
}
