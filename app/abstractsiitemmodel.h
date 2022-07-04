#ifndef ABSTRACTSIITEMMODEL_H
#define ABSTRACTSIITEMMODEL_H

#include <chunk.h>
#include <QAbstractItemModel>

class AbstractSIItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit AbstractSIItemModel(QObject *parent = nullptr);

  void SetChunk(si::Chunk *c);

protected:
  si::Chunk *GetChunkFromIndex(const QModelIndex &index) const;

  si::Chunk *root() const { return chunk_; }

private:
  si::Chunk *chunk_;

};

#endif // ABSTRACTSIITEMMODEL_H
