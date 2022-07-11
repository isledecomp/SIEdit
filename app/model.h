#ifndef MODEL_H
#define MODEL_H

#include <core.h>
#include <QAbstractItemModel>

class Model : public QAbstractItemModel
{
public:
  explicit Model(QObject *parent = nullptr);

  si::Core *GetCore() const { return core_; }
  void SetCore(si::Core *c);

  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex &index) const override;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
  si::Core *GetCoreFromIndex(const QModelIndex &index) const;

private:
  si::Core *core_;

};

#endif // MODEL_H
