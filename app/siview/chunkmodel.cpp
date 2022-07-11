#include "chunkmodel.h"

#include <iostream>

#define super Model

using namespace si;

ChunkModel::ChunkModel(QObject *parent) :
  super{parent}
{
}

int ChunkModel::columnCount(const QModelIndex &parent) const
{
  return kColCount;
}

QVariant ChunkModel::data(const QModelIndex &index, int role) const
{
  Chunk *c = static_cast<Chunk*>(GetCoreFromIndex(index));
  if (!c) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:

    switch (index.column()) {
    case kColType:
      // Convert 4-byte ID to QString
      return QString::fromLatin1(reinterpret_cast<const char *>(&c->id()), sizeof(uint32_t));
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
