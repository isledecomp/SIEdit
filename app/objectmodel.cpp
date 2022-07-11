#include "objectmodel.h"

#include <object.h>

#define super Model

using namespace si;

ObjectModel::ObjectModel(QObject *parent) :
  super{parent}
{
}

int ObjectModel::columnCount(const QModelIndex &parent) const
{
  return kColCount;
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
  Core *c = GetCoreFromIndex(index);

  switch (role) {
  case Qt::DisplayRole:

    switch (index.column()) {
    case kColIndex:
      if (Object *o = dynamic_cast<Object*>(c)) {
        if (!index.parent().isValid()) {
          return tr("%1:%2").arg(QString::number(index.row()), QString::number(o->id()));
        } else {
          return QString::number(o->id());
        }
      }
      break;
    case kColName:
      if (Object *o = dynamic_cast<Object*>(c)) {
        return QString::fromStdString(o->name());
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
    case kColName:
      return tr("Name");
    }
  }

  return super::headerData(section, orientation, role);
}
