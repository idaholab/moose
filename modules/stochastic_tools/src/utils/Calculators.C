//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Calculators.h"

#include <numeric>

#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"

#include "Statistics.h"
#include "MooseEnumItem.h"
#include "MooseError.h"
#include "MooseRandom.h"

namespace StochasticTools
{

MultiMooseEnum
makeCalculatorEnum()
{
  return MultiMooseEnum("min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6 stderr=7");
}

template <typename InType, typename OutType>
OutType
Calculator<InType, OutType>::compute(const InType & data, bool is_distributed)
{
  initialize();
  for (const auto & val : data)
    update(val);
  finalize(is_distributed);
  return get();
}

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Mean<InType, OutType>::initialize()
{
  _count = 0;
  _sum.zero();
}

template <typename InType, typename OutType>
void
Mean<InType, OutType>::update(const typename InType::value_type & val)
{
  _count++;
  _sum.add(val);
}

template <typename InType, typename OutType>
void
Mean<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.sum(_count);
    _sum.sum(this->_communicator);
  }
  if (_count > 0)
    _sum.divide(_count);
  else
    _sum.zero();
}

// MIN /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Min<InType, OutType>::initialize()
{
  _min.max();
}

template <typename InType, typename OutType>
void
Min<InType, OutType>::update(const typename InType::value_type & val)
{
  _min.min(val);
}

template <typename InType, typename OutType>
void
Min<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    _min.min(this->_communicator);
}

// MAX /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Max<InType, OutType>::initialize()
{
  _max.min();
}

template <typename InType, typename OutType>
void
Max<InType, OutType>::update(const typename InType::value_type & val)
{
  _max.max(val);
}

template <typename InType, typename OutType>
void
Max<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    _max.max(this->_communicator);
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Sum<InType, OutType>::initialize()
{
  _sum.zero();
}

template <typename InType, typename OutType>
void
Sum<InType, OutType>::update(const typename InType::value_type & val)
{
  _sum.add(val);
}

template <typename InType, typename OutType>
void
Sum<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    _sum.sum(this->_communicator);
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
StdDev<InType, OutType>::initialize()
{
  _count = 0;
  _sum.zero();
  _sum_of_square.zero();
}

template <typename InType, typename OutType>
void
StdDev<InType, OutType>::update(const typename InType::value_type & val)
{
  _count++;
  _sum.add(val);
  _sum_of_square.addPow(val, 2);
}

template <typename InType, typename OutType>
void
StdDev<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.sum(_count);
    _sum.sum(this->_communicator);
    _sum_of_square.sum(this->_communicator);
  }

  if (_count <= 1)
    _sum_of_square.zero();
  else
  {
    _sum.pow(2);
    _sum.divide(_count);
    _sum_of_square -= _sum.get();
    _sum_of_square.divide(_count - 1);
    _sum_of_square.sqrt();
  }
}

// STDERR //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
StdErr<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.sum(this->_count);
    this->_sum.sum(this->_communicator);
    this->_sum_of_square.sum(this->_communicator);
  }

  if (this->_count <= 1)
    this->_sum_of_square.zero();
  else
  {
    this->_sum.pow(2);
    this->_sum.divide(this->_count);
    this->_sum_of_square -= this->_sum.get();
    this->_sum_of_square.divide(this->_count - 1);
    this->_sum_of_square.divide(this->_count);
    this->_sum_of_square.sqrt();
  }
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Ratio<InType, OutType>::initialize()
{
  _min.max();
  _max.min();
}

template <typename InType, typename OutType>
void
Ratio<InType, OutType>::update(const typename InType::value_type & val)
{
  _min.min(val);
  _max.max(val);
}

template <typename InType, typename OutType>
void
Ratio<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    _min.min(this->_communicator);
    _max.max(this->_communicator);
  }
  _max /= _min.get();
}

// L2NORM //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::initialize()
{
  _l2_norm.zero();
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::update(const typename InType::value_type & val)
{
  _l2_norm.addPow(val, 2);
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    _l2_norm.sum(this->_communicator);
  _l2_norm.sqrt();
}

// makeCalculator //////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::unique_ptr<Calculator<InType, OutType>>
makeCalculator(const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  if (item == "min")
    return libmesh_make_unique<Min<InType, OutType>>(other, item);

  else if (item == "max")
    return libmesh_make_unique<Max<InType, OutType>>(other, item);

  else if (item == "sum")
    return libmesh_make_unique<Sum<InType, OutType>>(other, item);

  else if (item == "mean" || item == "average") // average is deprecated
    return libmesh_make_unique<Mean<InType, OutType>>(other, item);

  else if (item == "stddev")
    return libmesh_make_unique<StdDev<InType, OutType>>(other, item);

  else if (item == "stderr")
    return libmesh_make_unique<StdErr<InType, OutType>>(other, item);

  else if (item == "norm2")
    return libmesh_make_unique<L2Norm<InType, OutType>>(other, item);

  else if (item == "ratio")
    return libmesh_make_unique<Ratio<InType, OutType>>(other, item);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
