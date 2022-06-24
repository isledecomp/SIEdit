#ifndef CHUNKMODEL_H
#define CHUNKMODEL_H

#include <chunk.h>
#include <QAbstractItemModel>

class ChunkModel : public QAbstractItemModel
{
public:
  enum Columns
  {
    kColType,
    kColDesc,

    kColCount
  };

  explicit ChunkModel(QObject *parent = nullptr);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &index) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void SetChunk(Chunk *c);

private:
  Chunk *GetChunkFromIndex(const QModelIndex &index) const;

  Chunk *chunk_;

};

#endif // CHUNKMODEL_H
