//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseADWrapper.h"

#include "libmesh/auto_ptr.h"

void
MooseADWrapper<Real>::copyDualNumberToValue() const
{
  _val = _dual_number->value();
}

void
MooseADWrapper<Real>::initializeDual() const
{
  _dual_number = libmesh_make_unique<DualReal>(_val);
}

void
MooseADWrapper<VectorValue<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<VectorValue<DualReal>>(_val);
}

void
MooseADWrapper<VectorValue<Real>>::copyDualNumberToValue() const
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    _val(i) = (*_dual_number)(i).value();
}

void
MooseADWrapper<TensorValue<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<TensorValue<DualReal>>(_val);
}

void
MooseADWrapper<TensorValue<Real>>::copyDualNumberToValue() const
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<RankTwoTensorTempl<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<RankTwoTensorTempl<DualReal>>(_val);
}

void
MooseADWrapper<RankTwoTensorTempl<Real>>::copyDualNumberToValue() const
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<RankThreeTensorTempl<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<RankThreeTensorTempl<DualReal>>(_val);
}

void
MooseADWrapper<RankThreeTensorTempl<Real>>::copyDualNumberToValue() const
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
        _val(i, j, k) = (*_dual_number)(i, j, k).value();
}

void
MooseADWrapper<RankFourTensorTempl<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<RankFourTensorTempl<DualReal>>(_val);
}

void
MooseADWrapper<RankFourTensorTempl<Real>>::copyDualNumberToValue() const
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
        for (std::size_t l = 0; l < LIBMESH_DIM; ++l)
          _val(i, j, k, l) = (*_dual_number)(i, j, k, l).value();
}

void
MooseADWrapper<DenseVector<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<DenseVector<DualReal>>(_val.size());
}

void
MooseADWrapper<DenseVector<Real>>::copyValueToDualNumber() const
{
  _dual_number->resize(_val.size());
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
    (*_dual_number)(i) = _val(i);
}

void
MooseADWrapper<DenseVector<Real>>::copyDualNumberToValue() const
{
  _val.resize(_dual_number->size());
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
    _val(i) = (*_dual_number)(i).value();
}

void
MooseADWrapper<DenseMatrix<Real>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<DenseMatrix<DualReal>>(_val.m(), _val.n());
}

void
MooseADWrapper<DenseMatrix<Real>>::copyValueToDualNumber() const
{
  auto m = _val.m();
  auto n = _val.n();
  _dual_number->resize(m, n);
  for (std::size_t i = 0; i < m; ++i)
    for (std::size_t j = 0; j < n; ++j)
      (*_dual_number)(i, j) = _val(i, j);
}

void
MooseADWrapper<DenseMatrix<Real>>::copyDualNumberToValue() const
{
  _val.resize(_dual_number->m(), _dual_number->n());
  for (std::size_t i = 0; i < _dual_number->m(); ++i)
    for (std::size_t j = 0; j < _dual_number->n(); ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<std::vector<DenseVector<Real>>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<std::vector<DenseVector<DualReal>>>(_val.size());
}

void
MooseADWrapper<std::vector<DenseVector<Real>>>::copyValueToDualNumber() const
{
  _dual_number->resize(_val.size());
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
  {
    auto & val = _val[i];
    auto & dn = (*_dual_number)[i];
    dn.resize(val.size());
    for (std::size_t j = 0; j < val.size(); ++j)
      dn(j) = val(j);
  }
}

void
MooseADWrapper<std::vector<DenseVector<Real>>>::copyDualNumberToValue() const
{
  _val.resize(_dual_number->size());
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
  {
    _val[i].resize((*_dual_number)[i].size());
    for (std::size_t j = 0; j < (*_dual_number)[i].size(); ++j)
      _val[i](j) = (*_dual_number)[i](j).value();
  }
}

void
MooseADWrapper<std::vector<DenseMatrix<Real>>>::initializeDual() const
{
  _dual_number = libmesh_make_unique<std::vector<DenseMatrix<DualReal>>>(_val.size());
}

void
MooseADWrapper<std::vector<DenseMatrix<Real>>>::copyValueToDualNumber() const
{
  _dual_number->resize(_val.size());
  for (size_t h = 0; h < _val.size(); h++)
  {
    auto & val = _val[h];
    (*_dual_number)[h].resize(val.m(), val.n());
    for (std::size_t i = 0; i < val.m(); ++i)
      for (std::size_t j = 0; j < val.n(); ++j)
        (*_dual_number)[h](i, j) = val(i, j);
  }
}

void
MooseADWrapper<std::vector<DenseMatrix<Real>>>::copyDualNumberToValue() const
{
  _val.resize(_dual_number->size());
  for (size_t h = 0; h < _dual_number->size(); h++)
  {
    auto & val = (*_dual_number)[h];
    _val[h].resize(val.m(), val.n());
    for (std::size_t i = 0; i < val.m(); ++i)
      for (std::size_t j = 0; j < val.n(); ++j)
        _val[h](i, j) = val(i, j).value();
  }
}

