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
};
}
#endif
#endif
