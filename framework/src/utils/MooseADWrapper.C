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
