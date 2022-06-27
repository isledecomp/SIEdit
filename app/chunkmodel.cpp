#include "chunkmodel.h"

#include <data/riff.h>
#include <iostream>

#define super QAbstractItemModel

using namespace si;

ChunkModel::ChunkModel(QObject *parent) :
  super{parent},
  chunk_(nullptr)
{
}

int ChunkModel::columnCount(const QModelIndex &parent) const
{
  return kColCount;
}

QModelIndex ChunkModel::index(int row, int column, const QModelIndex &parent) const
{
  Chunk *c = GetChunkFromIndex(parent);
  if (!c) {
    return QModelIndex();
  }

  return createIndex(row, column, c->GetChildAt(row));
}

QModelIndex ChunkModel::parent(const QModelIndex &index) const
{
  Chunk *child = GetChunkFromIndex(index);
  if (!child) {
    return QModelIndex();
  }

  Chunk *parent = child->GetParent();
  if (!parent) {
    return QModelIndex();
  }

  Chunk *grandparent = parent->GetParent();
  if (!grandparent) {
    return QModelIndex();
  }

  size_t row = grandparent->IndexOfChild(parent);
  return createIndex(row, index.column(), parent);
}

int ChunkModel::rowCount(const QModelIndex &parent) const
{
  Chunk *c = GetChunkFromIndex(parent);
  if (!c) {
    return 0;
  }

  return c->GetChildCount();
}

QVariant ChunkModel::data(const QModelIndex &index, int role) const
{
  Chunk *c = GetChunkFromIndex(index);
  if (!c) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:

    switch (index.column()) {
    case kColType:
      // Convert 4-byte ID to QString
      return QString::fromLatin1(reinterpret_cast<const char *>(&c->id()), sizeof(u32));
    case kColOffset:
      return QStringLiteral("0x%1").arg(QString::number(c->offset(), 16).toUpper());
    case kColDesc:
      return QString::fromUtf8(c->GetTypeDescription());
    }

    break;
  }

  return QVariant();
}

QVariant ChunkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case kColType:
      return tr("Type");
    case kColOffset:
      return tr("Offset");
    case kColDesc:
      return tr("Description");
    }
  }

  return super::headerData(section, orientation, role);
}

void ChunkModel::SetChunk(si::Chunk *c)
{
  beginResetModel();
  chunk_ = c;
  endResetModel();
}

si::Chunk *ChunkModel::GetChunkFromIndex(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return chunk_;
  } else {
    return static_cast<Chunk*>(index.internalPointer());
  }
}
