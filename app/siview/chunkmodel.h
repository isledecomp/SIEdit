#ifndef CHUNKMODEL_H
#define CHUNKMODEL_H

#include <info.h>

#include "model.h"

class ChunkModel : public Model
{
  Q_OBJECT
public:
  enum Columns
  {
    kColType,
    kColOffset,
    kColSize,
    kColDesc,
    kColObjectID,

    kColCount
  };

  explicit ChunkModel(QObject *parent = nullptr);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

};

#endif // CHUNKMODEL_H
