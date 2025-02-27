#ifdef MFEM_ENABLED

#pragma once
#include <vector>
#include <memory>
#include <tuple>

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"

namespace MooseMFEM
{
/**
 * Factory class to create tracked objects. The objects may be either
 * of this class or any subclass. Iterator methods are provided to
 * allow access to all of the objects that have been created.
 */
template <class T>
class ObjectManager
{
public:
  using storage = typename std::vector<std::shared_ptr<T>>;
  using iter = typename storage::iterator;
  using const_iter = typename storage::const_iterator;
  using reverse_iter = typename storage::reverse_iterator;
  using const_reverse_iter = typename storage::const_reverse_iterator;

  template <class P, class... Args>
  std::shared_ptr<P> make(Args &&... args)
  {
    auto result = std::make_shared<P>(args...);
    this->_objects.push_back(result);
    return result;
  }

  iter begin() { return this->_objects.begin(); }
  iter end() { return this->_objects.end(); }
  const_iter cbegin() const { return this->_objects.cbegin(); }
  const_iter cend() const { return this->_objects.cend(); }
  reverse_iter rbegin() { return this->_objects.rbegin(); }
  reverse_iter rend() { return this->_objects.rend(); }
  const_reverse_iter crbegin() const { return this->_objects.crbegin(); }
  const_reverse_iter crend() const { return this->_objects.crend(); }

private:
  storage _objects;
};

using ScalarCoefficientManager = ObjectManager<mfem::Coefficient>;
using VectorCoefficientManager = ObjectManager<mfem::VectorCoefficient>;
using MatrixCoefficientManager = ObjectManager<mfem::MatrixCoefficient>;

}

#endif
