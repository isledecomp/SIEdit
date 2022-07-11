#ifndef CORE_H
#define CORE_H

#include <vector>

#include "common.h"

namespace si {

class Core
{
public:
  Core();
  LIBWEAVER_EXPORT virtual ~Core();

  typedef std::vector<Core*> Children;

  LIBWEAVER_EXPORT Core *GetParent() const { return parent_; }
  LIBWEAVER_EXPORT const Children &GetChildren() const { return children_; }

  bool FindParent(Core *p) const;

  LIBWEAVER_EXPORT void AppendChild(Core *Core);
  LIBWEAVER_EXPORT bool RemoveChild(Core *Core);
  LIBWEAVER_EXPORT size_t IndexOfChild(Core *Core);
  LIBWEAVER_EXPORT void InsertChild(size_t index, Core *Core);
  LIBWEAVER_EXPORT Core *RemoveChild(size_t index);
  LIBWEAVER_EXPORT Core *GetChildAt(size_t index) const { return children_.at(index); }
  LIBWEAVER_EXPORT size_t GetChildCount() const { return children_.size(); }

protected:
  void DeleteChildren();

private:
  // Disable copy
  Core(const Core& other);
  Core& operator=(const Core& other);

  Core *parent_;
  Children children_;

};

}

#endif // CORE_H
