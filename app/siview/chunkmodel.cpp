#include "chunkmodel.h"

#include <iostream>
#include <sitypes.h>

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
  Info *c = static_cast<Info*>(GetCoreFromIndex(index));
  if (!c) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:

    switch (index.column()) {
    case kColType:
      // Convert 4-byte ID to QString
      return QString::fromLatin1(reinterpret_cast<const char *>(&c->GetType()), sizeof(uint32_t));
    case kColOffset:
      return QStringLiteral("0x%1").arg(QString::number(c->GetOffset(), 16).toUpper());
    case kColSize:
      return QStringLiteral("0x%1").arg(QString::number(c->GetSize(), 16).toUpper());
    case kColDesc:
      return QString::fromUtf8(RIFF::GetTypeDescription(static_cast<RIFF::Type>(c->GetType())));
    case kColObjectID:
      uint32_t i = c->GetObjectID();
      if (i != Info::NULL_OBJECT_ID) {
        return QString::number(i);
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
    case kColSize:
      return tr("Size");
    case kColDesc:
      return tr("Description");
    case kColObjectID:
      return tr("Object ID");
    }
  }

  return super::headerData(section, orientation, role);
}
