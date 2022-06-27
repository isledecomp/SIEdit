#include "riff.h"

#include <data/riff.h>
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
  RIFF *riff = chunk->data().cast<RIFF>();

  QString s = QString::fromLatin1((const char *) &riff->dwID, sizeof(u32));
  id_edit_->setText(s);
}

void RIFFPanel::OnClosingData(Chunk *chunk)
{
  RIFF *riff = chunk->data().cast<RIFF>();

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

  riff->dwID = *(u32*)d.data();
}
