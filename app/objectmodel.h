#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <interleaf.h>

#include "model.h"

class ObjectModel : public Model
{
  Q_OBJECT
public:
  enum Columns {
    kColIndex,
    kColName,

    kColCount
  };

  explicit ObjectModel(QObject *parent = nullptr);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

};

#endif // OBJECTMODEL_H
