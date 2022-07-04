#include "abstractsiitemmodel.h"

using namespace si;

AbstractSIItemModel::AbstractSIItemModel(QObject *parent) :
  QAbstractItemModel{parent},
  chunk_(nullptr)
{

}

void AbstractSIItemModel::SetChunk(si::Chunk *c)
{
  beginResetModel();
  chunk_ = c;
  endResetModel();
}

si::Chunk *AbstractSIItemModel::GetChunkFromIndex(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return chunk_;
  } else {
    return static_cast<Chunk*>(index.internalPointer());
  }
}
