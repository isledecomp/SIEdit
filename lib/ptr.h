#ifndef PTR_H
#define PTR_H

namespace si {

/**
 * @brief Smart pointer implementation for versions of C++ < 11
 */
template <typename T>
class Ptr
{
public:
  Ptr(T *ptr = 0)
  {
    data_ = ptr;
    ref_count_ = new size_t;
    *ref_count_ = 1;
  }

  ~Ptr()
  {
    *ref_count_--;
    if (*ref_count_ == 0) {
      delete data_;
      delete ref_count_;
    }
  }

  Ptr(const Ptr<T> &other)
  {
    data_ = other.data_;
    ref_count_ = other.ref_count_;
    *ref_count_++;
  }

  Ptr<T> &operator=(const Ptr<T> &other)
  {
    if (this != other) {
      *ref_count_--;
      if (*ref_count_ == 0) {
        delete data_;
        delete ref_count_;
      }

      data_ = other.data_;
      ref_count_ = other.ref_count_;
      *ref_count_++;
    }
  }

private:
  T *data_;
  size_t *ref_count_;

};

}

#endif // PTR_H
