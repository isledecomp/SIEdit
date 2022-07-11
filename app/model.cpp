#include "model.h"

#define super QAbstractItemModel

Model::Model(QObject *parent) :
  super(parent),
  core_(nullptr)
{

}

void Model::SetCore(si::Core *c)
{
  beginResetModel();
  core_ = c;
  endResetModel();
}

si::Core *Model::GetCoreFromIndex(const QModelIndex &index) const
{
  if (!index.isValid()) {
    return core_;
  } else {
    return static_cast<si::Core*>(index.internalPointer());
  }
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
  si::Core *c = GetCoreFromIndex(parent);
  if (!c) {
    return QModelIndex();
  }

  return createIndex(row, column, c->GetChildAt(row));
}

QModelIndex Model::parent(const QModelIndex &index) const
{
  si::Core *child = GetCoreFromIndex(index);
  if (!child) {
    return QModelIndex();
  }

  si::Core *parent = child->GetParent();
  if (!parent) {
    return QModelIndex();
  }

  si::Core *grandparent = parent->GetParent();
  if (!grandparent) {
    return QModelIndex();
  }

  size_t row = grandparent->IndexOfChild(parent);
  return createIndex(int(row), 0, parent);
}

int Model::rowCount(const QModelIndex &parent) const
{
  si::Core *c = GetCoreFromIndex(parent);
  if (!c) {
    return 0;
  }

  return int(c->GetChildCount());
}
