#include "objectmodel.h"

#include <chunk.h>

#define super QAbstractItemModel

using namespace si;

ObjectModel::ObjectModel(QObject *parent) :
  super{parent}
{
}

int ObjectModel::columnCount(const QModelIndex &parent) const
{
  return kColCount;
}

QModelIndex ObjectModel::index(int row, int column, const QModelIndex &parent) const
{
  return createIndex(row, column, GetItem(row));
}

QModelIndex ObjectModel::parent(const QModelIndex &index) const
{
  return QModelIndex();
}

int ObjectModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    return 0;
  } else {
    Chunk *mxof = GetMxOf();
    if (!mxof) {
      return 0;
    }

    return int(mxof->data("Offsets").size() / sizeof(u32));
  }
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
  u32 offset;
  Chunk *c = GetItem(index.row(), &offset);

  switch (role) {
  case Qt::DisplayRole:

    switch (index.column()) {
    case kColIndex:
      return index.row();
    case kColOffset:
      return QStringLiteral("0x%1").arg(QString::number(offset, 16).toUpper());
    case kColName:
      if (c) {
        Chunk *mxob = c->FindChildWithType(Chunk::TYPE_MxOb);
        if (mxob) {
          return QString(mxob->data("Name"));
        }
      }
      break;
    }

    break;
  }

  return QVariant();
}

QVariant ObjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case kColIndex:
      return tr("Index");
    case kColOffset:
      return tr("Offset");
    case kColName:
      return tr("Name");
    }
  }

  return super::headerData(section, orientation, role);
}

Chunk *ObjectModel::GetMxOf() const
{
  Chunk *root = this->root();
  if (!root) {
    return NULL;
  }

  return root->FindChildWithType(Chunk::TYPE_MxOf);
}

Chunk *ObjectModel::GetItem(size_t index, si::u32 *offset_out) const
{
  Chunk *mxof = GetMxOf();
  if (!mxof) {
    return NULL;
  }

  const Data &offset_bytes = mxof->data("Offsets");
  if (index >= offset_bytes.size()/sizeof(u32)) {
    return NULL;
  }

  const u32 *offsets = reinterpret_cast<const u32*>(offset_bytes.data());
  u32 offset = offsets[index];
  if (offset_out) {
    *offset_out = offset;
  }

  if (offset == 0) {
    return NULL;
  } else {
    return root()->FindChildWithOffset(offset);
  }
}
