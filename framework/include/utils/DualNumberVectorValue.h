#ifndef DUALNUMBERVECTORVALUE_H
#define DUALNUMBERVECTORVALUE_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_METAPHYSICL

#include "libmesh/vector_value.h"
#include "metaphysicl/compare_types.h"
#include "metaphysicl/nddualnumber.h"

#include <utility>

namespace MetaPhysicL
{
template <typename T, bool reverseorder>
struct PlusType<libMesh::VectorValue<T>, libMesh::VectorValue<T>, reverseorder>
{
  typedef libMesh::VectorValue<T> supertype;
};

template <typename T, bool reverseorder>
struct MinusType<libMesh::VectorValue<T>, libMesh::VectorValue<T>, reverseorder>
{
  typedef libMesh::VectorValue<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<libMesh::VectorValue<T>, libMesh::VectorValue<T>, reverseorder>
{
  typedef Real supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<libMesh::VectorValue<T>, libMesh::TypeVector<T>, reverseorder>
{
  typedef Real supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<Real, libMesh::VectorValue<T>, reverseorder>
{
  typedef libMesh::VectorValue<T> supertype;
};

template <typename T>
struct DividesType<libMesh::VectorValue<T>, Real>
{
  typedef libMesh::VectorValue<T> supertype;
};

template <typename T, typename D>
class NotADuckDualNumber<VectorValue<T>, D> : public DualNumber<VectorValue<T>, D>
{
public:
  using DualNumber<VectorValue<T>, D>::DualNumber;

  NotADuckDualNumber() : DualNumber<VectorValue<T>, D>() {}

  template <typename T2, typename D2>
  NotADuckDualNumber(const NotADuckDualNumber<TypeVector<T2>, D2> & type_vector)
    : DualNumber<VectorValue<T>, D>(type_vector.value(), type_vector.derivatives())
  {
  }

  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  NotADuckDualNumber<VectorValue<T>, D> & operator=(const T2 & scalar)
  {
    this->value() = scalar;
    auto size = this->derivatives().size();
    for (decltype(size) i = 0; i < size; ++i)
      this->derivatives()[i] = scalar;
    return *this;
  }
  template <typename T2, typename D2>
  NotADuckDualNumber<VectorValue<T>, D> &
  operator=(const NotADuckDualNumber<TypeVector<T2>, D2> & vector)
  {
    this->value() = vector.value();
    auto size = this->derivatives().size();
    for (decltype(size) i = 0; i < size; ++i)
      this->derivatives()[i] = vector.derivatives()[i];
    return *this;
  }
};
}
#endif
#endif
