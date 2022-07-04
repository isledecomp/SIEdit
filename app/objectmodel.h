#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include "abstractsiitemmodel.h"

class ObjectModel : public AbstractSIItemModel
{
  Q_OBJECT
public:
  enum Columns {
    kColIndex,
    kColOffset,
    kColName,

    kColCount
  };

  explicit ObjectModel(QObject *parent = nullptr);

  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &index) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
  si::Chunk *GetMxOf() const;
  si::Chunk *GetItem(size_t index, si::u32 *offset_out = NULL) const;

};

#endif // OBJECTMODEL_H
