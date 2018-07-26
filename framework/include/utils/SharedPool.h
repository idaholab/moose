//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SHAREDPOOL_H
#define SHAREDPOOL_H

// System Includes
#include <stack>

namespace MooseUtils
{

/**
 *
 * Originally From https://stackoverflow.com/a/27837534/2042320
 *
 * friedmud added variadic templated perfect forwarding to acquire()
 */

template <class T>
class SharedPool
{
private:
  struct External_Deleter
  {
    explicit External_Deleter(std::weak_ptr<SharedPool<T> *> pool) : pool_(pool) {}

    void operator()(T * ptr)
    {
      if (auto pool_ptr = pool_.lock())
      {
        try
        {
          (*pool_ptr.get())->add(std::unique_ptr<T>{ptr});
          return;
        }
        catch (...)
        {
        }
      }
      std::default_delete<T>{}(ptr);
    }

  private:
    std::weak_ptr<SharedPool<T> *> pool_;
  };

public:
  using ptr_type = std::unique_ptr<T, External_Deleter>;

  SharedPool() : this_ptr_(new SharedPool<T> *(this)) {}
  virtual ~SharedPool() {}

  void add(std::unique_ptr<T> t) { pool_.push(std::move(t)); }

  template <typename... Args>
  ptr_type acquire(Args &&... args)
  {
    // assert(!pool_.empty());

    // If the pool is empty - create one
    if (pool_.empty())
      return std::move(ptr_type(new T(std::forward<Args>(args)...),
                                External_Deleter{std::weak_ptr<SharedPool<T> *>{this_ptr_}}));

    ptr_type tmp(pool_.top().release(),
                 External_Deleter{std::weak_ptr<SharedPool<T> *>{this_ptr_}});
    pool_.pop();
    return std::move(tmp);
  }

  bool empty() const { return pool_.empty(); }

  size_t size() const { return pool_.size(); }

private:
  std::shared_ptr<SharedPool<T> *> this_ptr_;
  std::stack<std::unique_ptr<T>> pool_;
};
}

#endif // SHAREDPOOL_H
