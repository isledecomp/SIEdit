#include "chunkmodel.h"

#include <iostream>

#define super AbstractSIItemModel

using namespace si;

ChunkModel::ChunkModel(QObject *parent) :
  super{parent}
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
  return createIndex(int(row), 0, parent);
}

int ChunkModel::rowCount(const QModelIndex &parent) const
{
  Chunk *c = GetChunkFromIndex(parent);
  if (!c) {
    return 0;
  }

  return int(c->GetChildCount());
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
    case kColObjectID:
      if (c->type() == Chunk::TYPE_MxOb) {
        return c->data("ID").toU32();
      } else if (c->type() == Chunk::TYPE_MxCh) {
        return c->data("Object").toU32();
      }
      break;
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
    case kColObjectID:
      return tr("Object ID");
    }
  }

  return super::headerData(section, orientation, role);
}
