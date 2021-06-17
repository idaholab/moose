//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// System Includes
#include <stack>
#include <memory>

namespace MooseUtils
{

template <class T, typename... Args>
auto
reset(int, T & obj, Args... args) -> decltype(obj.reset(args...), void())
{
  obj.reset(std::forward<Args>(args)...);
}

template <class T, typename... Args>
void
reset(double, T & /*obj*/, Args... /*args*/)
{
}

/**
 *
 * Originally From https://stackoverflow.com/a/27837534/2042320
 *
 * friedmud added variadic templated perfect forwarding to acquire()
 *
 * For an object to be resetable it needs to define a reset() function
 * that takes the same arguments as its constructor.
 */
template <class T>
class SharedPool
{
private:
  struct ExternalDeleter
  {
    explicit ExternalDeleter(std::weak_ptr<SharedPool<T> *> pool) : _pool(pool) {}

    void operator()(T * ptr)
    {
      if (auto _poolptr = _pool.lock())
      {
        try
        {
          (*_poolptr.get())->add(std::unique_ptr<T>{ptr});
          return;
        }
        catch (...)
        {
        }
      }
      std::default_delete<T>{}(ptr);
    }

  private:
    std::weak_ptr<SharedPool<T> *> _pool;
  };

public:
  typedef typename std::unique_ptr<T, ExternalDeleter> PtrType;

  SharedPool() : _this_ptr(new SharedPool<T> *(this)) {}
  virtual ~SharedPool() {}

  void add(std::unique_ptr<T> t) { _pool.push(std::move(t)); }

  template <typename... Args>
  PtrType acquire(Args &&... args)
  {
    // if the pool is empty - create one
    if (_pool.empty())
    {
      _num_created++;
      return std::move(PtrType(new T(std::forward<Args>(args)...),
                               ExternalDeleter{std::weak_ptr<SharedPool<T> *>{_this_ptr}}));
    }
    else
    {
      PtrType tmp(_pool.top().release(),
                  ExternalDeleter{std::weak_ptr<SharedPool<T> *>{_this_ptr}});
      _pool.pop();

      reset(1, *tmp, std::forward<Args>(args)...);

      return tmp;
    }
  }

  bool empty() const { return _pool.empty(); }

  size_t size() const { return _pool.size(); }

  size_t num_created() const { return _num_created; }

private:
  std::shared_ptr<SharedPool<T> *> _this_ptr;
  std::stack<std::unique_ptr<T>> _pool;

  size_t _num_created = 0;
};
}
