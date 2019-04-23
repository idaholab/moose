//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseADWrapper.h"

MooseADWrapper<Real>::MooseADWrapper(bool use_ad) : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<DualReal>();
}

const DualReal &
MooseADWrapper<Real>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<DualReal>(_val);
  else if (!_use_ad)
    _dual_number->value() = _val;
  return *_dual_number;
}

DualReal &
MooseADWrapper<Real>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<Real>::copyDualNumberToValue()
{
  _val = _dual_number->value();
}

void
MooseADWrapper<Real>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<DualReal>();
  _use_ad = use_ad;
}

MooseADWrapper<Real> &
MooseADWrapper<Real>::operator=(const MooseADWrapper<Real> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

MooseADWrapper<VectorValue<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<VectorValue<DualReal>>();
}

const VectorValue<DualReal> &
MooseADWrapper<VectorValue<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<VectorValue<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
      (*_dual_number)(i).value() = _val(i);
  return *_dual_number;
}

VectorValue<DualReal> &
MooseADWrapper<VectorValue<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<VectorValue<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    _val(i) = (*_dual_number)(i).value();
}

void
MooseADWrapper<VectorValue<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<VectorValue<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<VectorValue<Real>> &
MooseADWrapper<VectorValue<Real>>::operator=(const MooseADWrapper<VectorValue<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

MooseADWrapper<TensorValue<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<TensorValue<DualReal>>();
}

const TensorValue<DualReal> &
MooseADWrapper<TensorValue<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<TensorValue<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
      for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
        (*_dual_number)(i, j).value() = _val(i, j);
  return *_dual_number;
}

TensorValue<DualReal> &
MooseADWrapper<TensorValue<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<TensorValue<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<TensorValue<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<TensorValue<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<TensorValue<Real>> &
MooseADWrapper<TensorValue<Real>>::operator=(const MooseADWrapper<TensorValue<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

MooseADWrapper<RankTwoTensorTempl<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<RankTwoTensorTempl<DualReal>>();
}

const RankTwoTensorTempl<DualReal> &
MooseADWrapper<RankTwoTensorTempl<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<RankTwoTensorTempl<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
      for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
        (*_dual_number)(i, j).value() = _val(i, j);
  return *_dual_number;
}

RankTwoTensorTempl<DualReal> &
MooseADWrapper<RankTwoTensorTempl<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<RankTwoTensorTempl<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<RankTwoTensorTempl<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<RankTwoTensorTempl<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<RankTwoTensorTempl<Real>> &
MooseADWrapper<RankTwoTensorTempl<Real>>::
operator=(const MooseADWrapper<RankTwoTensorTempl<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

// RankThreeTensor
MooseADWrapper<RankThreeTensorTempl<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<RankThreeTensorTempl<DualReal>>();
}

const RankThreeTensorTempl<DualReal> &
MooseADWrapper<RankThreeTensorTempl<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<RankThreeTensorTempl<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
      for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
        for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
          (*_dual_number)(i, j, k).value() = _val(i, j, k);
  return *_dual_number;
}

RankThreeTensorTempl<DualReal> &
MooseADWrapper<RankThreeTensorTempl<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<RankThreeTensorTempl<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
        _val(i, j, k) = (*_dual_number)(i, j, k).value();
}

void
MooseADWrapper<RankThreeTensorTempl<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<RankThreeTensorTempl<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<RankThreeTensorTempl<Real>> &
MooseADWrapper<RankThreeTensorTempl<Real>>::
operator=(const MooseADWrapper<RankThreeTensorTempl<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

// RankFourTensor
MooseADWrapper<RankFourTensorTempl<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<RankFourTensorTempl<DualReal>>();
}

const RankFourTensorTempl<DualReal> &
MooseADWrapper<RankFourTensorTempl<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<RankFourTensorTempl<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
      for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
        for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
          for (std::size_t l = 0; l < LIBMESH_DIM; ++l)
            (*_dual_number)(i, j, k, l).value() = _val(i, j, k, l);
  return *_dual_number;
}

RankFourTensorTempl<DualReal> &
MooseADWrapper<RankFourTensorTempl<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<RankFourTensorTempl<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < LIBMESH_DIM; ++i)
    for (std::size_t j = 0; j < LIBMESH_DIM; ++j)
      for (std::size_t k = 0; k < LIBMESH_DIM; ++k)
        for (std::size_t l = 0; l < LIBMESH_DIM; ++l)
          _val(i, j, k, l) = (*_dual_number)(i, j, k, l).value();
}

void
MooseADWrapper<RankFourTensorTempl<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<RankFourTensorTempl<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<RankFourTensorTempl<Real>> &
MooseADWrapper<RankFourTensorTempl<Real>>::
operator=(const MooseADWrapper<RankFourTensorTempl<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    *_dual_number = 0;
  return *this;
}

MooseADWrapper<DenseVector<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<DenseVector<DualReal>>();
}

const DenseVector<DualReal> &
MooseADWrapper<DenseVector<Real>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<DenseVector<DualReal>>(_val);
  else if (!_use_ad)
    for (std::size_t i = 0; i < _dual_number->size(); ++i)
      (*_dual_number)(i).value() = _val(i);
  return *_dual_number;
}

DenseVector<DualReal> &
MooseADWrapper<DenseVector<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<DenseVector<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
    _val(i) = (*_dual_number)(i).value();
}

void
MooseADWrapper<DenseVector<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<DenseVector<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<DenseVector<Real>> &
MooseADWrapper<DenseVector<Real>>::operator=(const MooseADWrapper<DenseVector<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    // I don't know why we do this, but other code does it - ask Alex.
    for (size_t i = 0; i < _dual_number->size(); ++i)
      (*_dual_number)(i) = 0;
  return *this;
}

MooseADWrapper<DenseMatrix<Real>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<DenseMatrix<DualReal>>();
}

const DenseMatrix<DualReal> &
MooseADWrapper<DenseMatrix<Real>>::dn(bool) const
{
  auto m = _val.m();
  auto n = _val.n();
  if (!_dual_number)
  {
    _dual_number = libmesh_make_unique<DenseMatrix<DualReal>>(m, n);
    for (std::size_t i = 0; i < m; ++i)
      for (std::size_t j = 0; j < n; ++j)
        (*_dual_number)(i, j).value() = _val(i, j);
  }
  else if (!_use_ad)
    for (std::size_t i = 0; i < m; ++i)
      for (std::size_t j = 0; j < n; ++j)
        (*_dual_number)(i, j).value() = _val(i, j);
  return *_dual_number;
}

DenseMatrix<DualReal> &
MooseADWrapper<DenseMatrix<Real>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<DenseMatrix<Real>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < _dual_number->m(); ++i)
    for (std::size_t j = 0; j < _dual_number->n(); ++j)
      _val(i, j) = (*_dual_number)(i, j).value();
}

void
MooseADWrapper<DenseMatrix<Real>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<DenseMatrix<DualReal>>();
  _use_ad = use_ad;
}

MooseADWrapper<DenseMatrix<Real>> &
MooseADWrapper<DenseMatrix<Real>>::operator=(const MooseADWrapper<DenseMatrix<Real>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    // I don't know why we do this, but other code does it - ask Alex.
    for (std::size_t i = 0; i < _dual_number->m(); ++i)
      for (std::size_t j = 0; j < _dual_number->n(); ++j)
        (*_dual_number)(i, j) = 0;
  return *this;
}

MooseADWrapper<std::vector<DenseMatrix<Real>>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<std::vector<DenseMatrix<DualReal>>>();
}

const std::vector<DenseMatrix<DualReal>> &
MooseADWrapper<std::vector<DenseMatrix<Real>>>::dn(bool) const
{
  if (!_dual_number)
  {
    _dual_number = libmesh_make_unique<std::vector<DenseMatrix<DualReal>>>(_val.size());
    for (size_t h = 0; h < _val.size(); h++)
    {
      auto & val = _val[h];
      (*_dual_number)[h].resize(val.m(), val.n());
      for (std::size_t i = 0; i < val.m(); ++i)
        for (std::size_t j = 0; j < val.n(); ++j)
          (*_dual_number)[h](i, j).value() = val(i, j);
    }
  }
  else if (!_use_ad)
  {
    assert(_dual_number.get());
    assert(_dual_number->size() == _val.size());
    for (size_t h = 0; h < _val.size(); h++)
    {
      auto & val = _val[h];
      for (std::size_t i = 0; i < val.m(); ++i)
        for (std::size_t j = 0; j < val.n(); ++j)
          (*_dual_number)[h](i, j).value() = val(i, j);
    }
  }
  assert(_dual_number.get());
  return *_dual_number;
}

std::vector<DenseMatrix<DualReal>> &
MooseADWrapper<std::vector<DenseMatrix<Real>>>::dn(bool)
{
  assert(_dual_number.get());
  return *_dual_number;
}

void
MooseADWrapper<std::vector<DenseMatrix<Real>>>::copyDualNumberToValue()
{
  assert(_dual_number.get());
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

void
MooseADWrapper<std::vector<DenseMatrix<Real>>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_dual_number)
    _dual_number = libmesh_make_unique<std::vector<DenseMatrix<DualReal>>>(_val.size());
  _use_ad = use_ad;
}

MooseADWrapper<std::vector<DenseMatrix<Real>>> &
MooseADWrapper<std::vector<DenseMatrix<Real>>>::
operator=(const MooseADWrapper<std::vector<DenseMatrix<Real>>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    // I don't know why we do this, but other code does it - ask Alex.
    for (size_t h = 0; h < _dual_number->size(); h++)
    {
      auto & val = (*_dual_number)[h];
      for (std::size_t i = 0; i < val.m(); ++i)
        for (std::size_t j = 0; j < val.n(); ++j)
          val(i, j) = 0;
    }
  return *this;
}

MooseADWrapper<std::vector<DenseVector<Real>>>::MooseADWrapper(bool use_ad)
  : _use_ad(use_ad), _val(), _dual_number(nullptr)
{
  if (_use_ad)
    _dual_number = libmesh_make_unique<std::vector<DenseVector<DualReal>>>();
}

const std::vector<DenseVector<DualReal>> &
MooseADWrapper<std::vector<DenseVector<Real>>>::dn(bool) const
{
  if (!_dual_number)
    _dual_number = libmesh_make_unique<std::vector<DenseVector<DualReal>>>(_val.size());
  else if (!_use_ad)
    for (std::size_t i = 0; i < _dual_number->size(); ++i)
    {
      auto & val = _val[i];
      auto & dn = (*_dual_number)[i];
      dn.resize(val.size());
      for (std::size_t j = 0; j < val.size(); ++j)
        dn(j).value() = val(j);
    }
  return *_dual_number;
}

std::vector<DenseVector<DualReal>> &
MooseADWrapper<std::vector<DenseVector<Real>>>::dn(bool)
{
  return *_dual_number;
}

void
MooseADWrapper<std::vector<DenseVector<Real>>>::copyDualNumberToValue()
{
  for (std::size_t i = 0; i < _dual_number->size(); ++i)
    for (std::size_t j = 0; j < (*_dual_number)[i].size(); ++j)
      _val[i](j) = (*_dual_number)[i](j).value();
}

void
MooseADWrapper<std::vector<DenseVector<Real>>>::markAD(bool use_ad)
{
  if (!use_ad && _use_ad)
    _dual_number = nullptr;
  else if (use_ad && !_use_ad)
    _dual_number = libmesh_make_unique<std::vector<DenseVector<DualReal>>>();
  _use_ad = use_ad;
}

MooseADWrapper<std::vector<DenseVector<Real>>> &
MooseADWrapper<std::vector<DenseVector<Real>>>::
operator=(const MooseADWrapper<std::vector<DenseVector<Real>>> & rhs)
{
  _val = rhs._val;
  if (_dual_number && rhs._dual_number)
    *_dual_number = *rhs._dual_number;
  else if (_dual_number)
    // I don't know why we do this, but other code does it - ask Alex.
    for (std::size_t i = 0; i < _dual_number->size(); ++i)
      for (std::size_t j = 0; j < (*_dual_number)[i].size(); ++j)
        (*_dual_number)[i](j) = 0;
  return *this;
}
