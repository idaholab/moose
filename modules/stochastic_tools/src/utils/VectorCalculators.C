//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCalculators.h"
#include "BootstrapCalculators.h"

namespace StochasticTools
{

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::divide(dof_id_type num)
{
  for (auto & val : _value)
    val /= static_cast<T2>(num);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::pow(int p)
{
  for (auto & val : _value)
    val = MathUtils::pow(val, p);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::sqrt()
{
  for (auto & val : _value)
    val = std::sqrt(val);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::add(const std::vector<T1> & a)
{
  if (a.size() > _value.size())
    _value.resize(a.size(), T2());
  for (const auto & i : index_range(a))
    _value[i] += static_cast<T2>(a[i]);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::addPow(const std::vector<T1> & a, int p)
{
  if (a.size() > _value.size())
    _value.resize(a.size(), T2());
  for (const auto & i : index_range(a))
    _value[i] += MathUtils::pow(static_cast<T2>(a[i]), p);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::min(const std::vector<T1> & a)
{
  if (a.size() > _value.size())
    _value.resize(a.size(), std::numeric_limits<T2>::max());
  for (const auto & i : index_range(a))
    _value[i] = std::min(static_cast<T2>(a[i]), _value[i]);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::max(const std::vector<T1> & a)
{
  if (a.size() > _value.size())
    _value.resize(a.size(), std::numeric_limits<T2>::min());
  for (const auto & i : index_range(a))
    _value[i] = std::max(static_cast<T2>(a[i]), _value[i]);
}

template <typename T1, typename T2>
CalculatorValue<std::vector<T1>, std::vector<T2>> &
CalculatorValue<std::vector<T1>, std::vector<T2>>::operator-=(const std::vector<T2> & b)
{
  if (b.size() > _value.size())
    _value.resize(b.size(), T2());
  for (const auto & i : index_range(b))
    _value[i] -= b[i];
  return *this;
}

template <typename T1, typename T2>
CalculatorValue<std::vector<T1>, std::vector<T2>> &
CalculatorValue<std::vector<T1>, std::vector<T2>>::operator/=(const std::vector<T2> & b)
{
  if (b.size() > _value.size())
    _value.resize(b.size(), T2());
  for (const auto & i : index_range(b))
    _value[i] /= b[i];
  return *this;
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::sum(const libMesh::Parallel::Communicator & comm)
{
  auto sz = _value.size();
  comm.max(sz);
  _value.resize(sz, T2());
  comm.sum(_value);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::min(const libMesh::Parallel::Communicator & comm)
{
  auto sz = _value.size();
  comm.max(sz);
  _value.resize(sz, std::numeric_limits<T2>::max());
  comm.min(_value);
}

template <typename T1, typename T2>
void
CalculatorValue<std::vector<T1>, std::vector<T2>>::max(const libMesh::Parallel::Communicator & comm)
{
  auto sz = _value.size();
  comm.max(sz);
  _value.resize(sz, std::numeric_limits<T2>::min());
  comm.max(_value);
}

createCalculators(std::vector<std::vector<Real>>, std::vector<Real>);
createBootstrapCalculators(std::vector<std::vector<Real>>, std::vector<Real>);

} // StocasticTools namespace
