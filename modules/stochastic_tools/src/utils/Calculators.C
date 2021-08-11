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
  return MultiMooseEnum("min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6 stderr=7 median=8");
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

// MEDIAN //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Median<InType, OutType>::initialize()
{
  _storage.clear();
}

template <typename InType, typename OutType>
void
Median<InType, OutType>::update(const typename InType::value_type & val)
{
  _storage.emplace_back();
  _storage.back().add(val);
}

template <typename InType, typename OutType>
void
Median<InType, OutType>::finalize(bool is_distributed)
{
  // Make sure we aren't doing anything silly like taking the median of an empty vector
  _median.zero();
  auto count = _storage.size();
  if (is_distributed)
    this->_communicator.sum(count);
  if (count == 0)
    return;

  if (!is_distributed || this->n_processors() == 1)
  {
    std::sort(_storage.begin(),
              _storage.end(),
              [](const CValue<InType, OutType> & a, const CValue<InType, OutType> & b) {
                return a.less_than(b.get());
              });
    if (count % 2)
      _median += _storage[count / 2].get();
    else
    {
      _median += _storage[count / 2].get();
      _median += _storage[count / 2 - 1].get();
      _median.divide(2);
    }
    return;
  }

  dof_id_type kgt = count % 2 ? (count / 2) : (count / 2 - 1);
  dof_id_type klt = kgt;
  while (true)
  {
    // Gather all sizes and figure out current number of values
    std::vector<std::size_t> sz = {_storage.size()};
    this->_communicator.allgather(sz);
    dof_id_type n = std::accumulate(sz.begin(), sz.end(), 0);

    // Choose the first value for the first processor with values
    _median.zero();
    for (const auto & i : index_range(sz))
      if (sz[i])
      {
        if (this->processor_id() == i)
          _median += _storage[0].get();
        _median.broadcast(this->_communicator, i);
        break;
      }

    // Count number of values greater than, less than, and equal to _median
    std::vector<dof_id_type> m(3, 0);
    for (const auto & val : _storage)
    {
      if (_median.less_than(val.get()))
        m[0]++;
      else if (val.less_than(_median.get()))
        m[1]++;
    }
    this->_communicator.sum(m);
    m[2] = n - m[0] - m[1];

    // Remove greater than equal to
    if ((m[0] + m[2]) <= kgt)
    {
      _storage.erase(std::remove_if(_storage.begin(),
                                    _storage.end(),
                                    [this](const CValue<InType, OutType> & val) {
                                      return !val.less_than(_median.get());
                                    }),
                     _storage.end());
      kgt -= m[0] + m[2];
    }
    // Remove less than equal to
    else if ((m[1] + m[2]) <= klt)
    {
      _storage.erase(std::remove_if(_storage.begin(),
                                    _storage.end(),
                                    [this](const CValue<InType, OutType> & val) {
                                      return !_median.less_than(val.get());
                                    }),
                     _storage.end());
      klt -= m[1] + m[2];
    }
    // If the number of points is odd, then we've found it
    else if (count % 2)
      break;
    // Get average of the two middle numbers
    else
    {
      CValue<InType, OutType> num2;
      // Find the next greater than
      if (m[0] > kgt)
      {
        num2.max();
        for (const auto & val : _storage)
          if (_median.less_than(val.get()) && val.less_than(num2.get()))
          {
            num2.zero();
            num2 += val.get();
          }
        num2.min(this->_communicator);
      }
      // Find the next less than
      else if (m[1] > klt)
      {
        num2.min();
        for (const auto & val : _storage)
          if (val.less_than(_median.get()) && num2.less_than(val.get()))
          {
            num2.zero();
            num2 += val.get();
          }
        num2.max(this->_communicator);
      }
      // Otherwise we know the other number is equal
      else
        num2 += _median.get();

      _median += num2.get();
      _median.divide(2);
      break;
    }
  }
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

  else if (item == "median")
    return libmesh_make_unique<Median<InType, OutType>>(other, item);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
