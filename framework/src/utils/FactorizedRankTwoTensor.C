//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FactorizedRankTwoTensor.h"

template <typename T>
void
FactorizedRankTwoTensorTempl<T>::print(std::ostream & stm) const
{
  this->get().print(stm);
}

template <typename T>
FactorizedRankTwoTensorTempl<T>
FactorizedRankTwoTensorTempl<T>::rotated(const RankTwoTensorTempl<typename T::value_type> & R) const
{
  return FactorizedRankTwoTensorTempl<T>(_eigvals, R * _eigvecs);
}

template <typename T>
FactorizedRankTwoTensorTempl<T>
FactorizedRankTwoTensorTempl<T>::transpose() const
{
  return *this;
}

template <typename T>
FactorizedRankTwoTensorTempl<T> &
FactorizedRankTwoTensorTempl<T>::operator=(const FactorizedRankTwoTensorTempl<T> & A)
{
  _eigvals = A._eigvals;
  _eigvecs = A._eigvecs;
  return *this;
}

template <typename T>
FactorizedRankTwoTensorTempl<T> &
FactorizedRankTwoTensorTempl<T>::operator=(const T & A)
{
  A.symmetricEigenvaluesEigenvectors(_eigvals, _eigvecs);
  return *this;
}

template <typename T>
FactorizedRankTwoTensorTempl<T> &
FactorizedRankTwoTensorTempl<T>::operator*=(const typename T::value_type & a)
{
  for (auto & eigval : _eigvals)
    eigval *= a;
  return *this;
}

template <typename T>
FactorizedRankTwoTensorTempl<T> &
FactorizedRankTwoTensorTempl<T>::operator/=(const typename T::value_type & a)
{
  for (auto & eigval : _eigvals)
    eigval /= a;
  return *this;
}

template <typename T>
bool
FactorizedRankTwoTensorTempl<T>::operator==(const T & A) const
{
  T me = get();
  return me == A;
}

template <typename T>
bool
FactorizedRankTwoTensorTempl<T>::operator==(const FactorizedRankTwoTensorTempl<T> & A) const
{
  T me = get();
  T you = A.get();
  return me == you;
}

template <typename T>
FactorizedRankTwoTensorTempl<T>
FactorizedRankTwoTensorTempl<T>::inverse() const
{
  return FactorizedRankTwoTensorTempl<T>({1 / _eigvals[0], 1 / _eigvals[1], 1 / _eigvals[2]},
                                         _eigvecs);
}

template <typename T>
void
FactorizedRankTwoTensorTempl<T>::addIa(const typename T::value_type & a)
{
  for (auto & eigval : _eigvals)
    eigval += a;
}

template <typename T>
typename T::value_type
FactorizedRankTwoTensorTempl<T>::trace() const
{
  return _eigvals[0] + _eigvals[1] + _eigvals[2];
}

template <typename T>
typename T::value_type
FactorizedRankTwoTensorTempl<T>::det() const
{
  return _eigvals[0] * _eigvals[1] * _eigvals[2];
}

template class FactorizedRankTwoTensorTempl<RankTwoTensor>;
template class FactorizedRankTwoTensorTempl<ADRankTwoTensor>;
template class FactorizedRankTwoTensorTempl<SymmetricRankTwoTensor>;
template class FactorizedRankTwoTensorTempl<ADSymmetricRankTwoTensor>;
