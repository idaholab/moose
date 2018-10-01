#ifndef DUALNUMBERTENSORVALUE_H
#define DUALNUMBERTENSORVALUE_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_METAPHYSICL

#include "libmesh/tensor_value.h"
#include "metaphysicl/compare_types.h"
#include "metaphysicl/nddualnumber.h"

namespace MetaPhysicL
{

template <typename T, bool reverseorder>
struct PlusType<libMesh::TensorValue<T>, libMesh::TensorValue<T>, reverseorder>
{
  typedef libMesh::TensorValue<T> supertype;
};

template <typename T, bool reverseorder>
struct MinusType<libMesh::TensorValue<T>, libMesh::TensorValue<T>, reverseorder>
{
  typedef libMesh::TensorValue<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<libMesh::TensorValue<T>, libMesh::TensorValue<T>, reverseorder>
{
  typedef libMesh::TensorValue<T> supertype;
};

template <typename T, bool reverseorder>
struct MultipliesType<Real, libMesh::TensorValue<T>, reverseorder>
{
  typedef libMesh::TensorValue<T> supertype;
};

template <typename T>
struct MultipliesType<libMesh::TensorValue<T>, libMesh::TypeVector<T>>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T>
struct MultipliesType<libMesh::TensorValue<T>, libMesh::VectorValue<T>>
{
  typedef libMesh::TypeVector<T> supertype;
};

template <typename T>
struct DividesType<libMesh::TensorValue<T>, Real>
{
  typedef libMesh::TensorValue<T> supertype;
};

template <typename T, typename D>
class NotADuckDualNumber<TensorValue<T>, D> : public DualNumber<TensorValue<T>, D>
{
public:
  using DualNumber<TensorValue<T>, D>::DualNumber;

  NotADuckDualNumber() : DualNumber<TensorValue<T>, D>() {}

  template <typename T2, typename D2>
  NotADuckDualNumber(const NotADuckDualNumber<TypeTensor<T2>, D2> & type_tensor)
    : DualNumber<TensorValue<T>, D>(type_tensor.value(), type_tensor.derivatives())
  {
  }

  template <typename T2, typename std::enable_if<ScalarTraits<T2>::value, int>::type = 0>
  NotADuckDualNumber<TensorValue<T>, D> & operator=(const T2 & scalar)
  {
    this->value() = scalar;
    auto size = this->derivatives().size();
    for (decltype(size) i = 0; i < size; ++i)
      this->derivatives()[i] = scalar;
    return *this;
  }
  template <typename T2, typename D2>
  NotADuckDualNumber<TensorValue<T>, D> &
  operator=(const NotADuckDualNumber<TypeTensor<T2>, D2> & tensor)
  {
    this->value() = tensor.value();
    auto size = this->derivatives().size();
    for (decltype(size) i = 0; i < size; ++i)
      this->derivatives()[i] = tensor.derivatives()[i];
    return *this;
  }
};
}
#endif
#endif
