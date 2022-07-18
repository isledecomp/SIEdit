#ifndef CORE_H
#define CORE_H

#include <algorithm>
#include <vector>

#include "types.h"

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

  void AppendChild(Core *Core);
  bool RemoveChild(Core *Core);
  void InsertChild(size_t index, Core *Core);
  Core *RemoveChild(size_t index);

  LIBWEAVER_EXPORT size_t IndexOfChild(Core *Core) const;
  LIBWEAVER_EXPORT Core *GetChildAt(size_t index) const { return children_.at(index); }
  LIBWEAVER_EXPORT size_t GetChildCount() const { return children_.size(); }
  LIBWEAVER_EXPORT bool HasChildren() const { return !children_.empty(); }
  LIBWEAVER_EXPORT bool ContainsChild(Core *child) const { return std::find(children_.begin(), children_.end(), child) != children_.end(); }

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
