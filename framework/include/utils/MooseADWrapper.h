#ifndef MOOSEADWRAPPER_H
#define MOOSEADWRAPPER_H

#include "MooseTypes.h"
#include "MooseError.h"
#include "RankTwoTensor.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

#include <typeinfo>

template <typename T>
class MooseADWrapper
{
public:
  MooseADWrapper() : _val() {}

  typedef T DNType;

  const T & value() const { return _val; }

  T & value() { return _val; }

  const T & dn(bool requested_by_user = true) const
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return _val;
  }

  T & dn(bool requested_by_user = true)
  {
    if (requested_by_user)
      mooseError("Type ",
                 typeid(T).name(),
                 " does not currently support automatic differentiation. Consider using a regular "
                 "material property (declareProperty, getMaterialProperty) instead.");
    return _val;
  }

  void copyValueToDualNumber() {}
  void copyDualNumberToValue() {}

private:
  T _val;
};

template <>
class MooseADWrapper<Real>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef ADReal DNType;

  const Real & value() const { return _val; }

  Real & value() { return _val; }

  const ADReal & dn(bool = true) const { return _dual_number; }

  ADReal & dn(bool = true) { return _dual_number; }

  void copyValueToDualNumber() { _dual_number.value() = _val; }
  void copyDualNumberToValue() { _val = _dual_number.value(); }

private:
  Real _val;
  ADReal _dual_number;
};

template <>
class MooseADWrapper<libMesh::VectorValue<Real>>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef libMesh::VectorValue<ADReal> DNType;

  const libMesh::VectorValue<Real> & value() const { return _val; }

  libMesh::VectorValue<Real> & value() { return _val; }

  const libMesh::VectorValue<ADReal> & dn(bool = true) const { return _dual_number; }

  libMesh::VectorValue<ADReal> & dn(bool = true) { return _dual_number; }

  void copyValueToDualNumber()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      _dual_number(i).value() = _val(i);
  }
  void copyDualNumberToValue()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      _val(i) = _dual_number(i).value();
  }

private:
  libMesh::VectorValue<Real> _val;
  libMesh::VectorValue<ADReal> _dual_number;
};

template <>
class MooseADWrapper<libMesh::TensorValue<Real>>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef libMesh::TensorValue<ADReal> DNType;

  const libMesh::TensorValue<Real> & value() const { return _val; }

  libMesh::TensorValue<Real> & value() { return _val; }

  const libMesh::TensorValue<ADReal> & dn(bool = true) const { return _dual_number; }

  libMesh::TensorValue<ADReal> & dn(bool = true) { return _dual_number; }

  void copyValueToDualNumber()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      for (decltype(LIBMESH_DIM) j = 0; j < LIBMESH_DIM; ++j)
        _dual_number(i, j).value() = _val(i, j);
  }
  void copyDualNumberToValue()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      for (decltype(LIBMESH_DIM) j = 0; j < LIBMESH_DIM; ++j)
        _val(i, j) = _dual_number(i, j).value();
  }

private:
  libMesh::TensorValue<Real> _val;
  libMesh::TensorValue<ADReal> _dual_number;
};

template <>
class MooseADWrapper<RankTwoTensorTempl<Real>>
{
public:
  MooseADWrapper() : _val(), _dual_number() {}

  typedef RankTwoTensorTempl<ADReal> DNType;

  const RankTwoTensorTempl<Real> & value() const { return _val; }

  RankTwoTensorTempl<Real> & value() { return _val; }

  const RankTwoTensorTempl<ADReal> & dn(bool = true) const { return _dual_number; }

  RankTwoTensorTempl<ADReal> & dn(bool = true) { return _dual_number; }

  void copyValueToDualNumber()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      for (decltype(LIBMESH_DIM) j = 0; j < LIBMESH_DIM; ++j)
        _dual_number(i, j).value() = _val(i, j);
  }
  void copyDualNumberToValue()
  {
    for (decltype(LIBMESH_DIM) i = 0; i < LIBMESH_DIM; ++i)
      for (decltype(LIBMESH_DIM) j = 0; j < LIBMESH_DIM; ++j)
        _val(i, j) = _dual_number(i, j).value();
  }

private:
  RankTwoTensorTempl<Real> _val;
  RankTwoTensorTempl<ADReal> _dual_number;
};

#endif // MOOSEADWRAPPER_H
