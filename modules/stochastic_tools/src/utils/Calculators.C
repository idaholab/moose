//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Mean<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_count = data.size();
  auto local_sum = std::accumulate(data.begin(), data.end(), OutType());
  if (is_distributed)
  {
    this->_communicator.sum(local_count);
    this->_communicator.sum(local_sum);
  }
  return local_count == 0 ? 0.0 : local_sum / local_count;
}

template <typename InType, typename OutType>
void
Mean<InType, OutType>::initialize()
{
  _count = 0;
  _sum = OutType();
}

template <typename InType, typename OutType>
void
Mean<InType, OutType>::update(const typename InType::value_type & val)
{
  _count++;
  _sum += val;
}

template <typename InType, typename OutType>
void
Mean<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.sum(_count);
    this->_communicator.sum(_sum);
  }
}

// MIN /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Min<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_min = data.empty() ? std::numeric_limits<OutType>::max()
                                : *std::min_element(data.begin(), data.end());
  if (is_distributed)
    this->_communicator.min(local_min);
  return local_min;
}

template <typename InType, typename OutType>
void
Min<InType, OutType>::initialize()
{
  _min = std::numeric_limits<OutType>::max();
}

template <typename InType, typename OutType>
void
Min<InType, OutType>::update(const typename InType::value_type & val)
{
  _min = std::min(_min, static_cast<OutType>(val));
}

template <typename InType, typename OutType>
void
Min<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.min(_min);
}

// MAX /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Max<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_max = data.empty() ? std::numeric_limits<OutType>::min()
                                : *std::max_element(data.begin(), data.end());
  if (is_distributed)
    this->_communicator.max(local_max);
  return local_max;
}

template <typename InType, typename OutType>
void
Max<InType, OutType>::initialize()
{
  _max = std::numeric_limits<OutType>::min();
}

template <typename InType, typename OutType>
void
Max<InType, OutType>::update(const typename InType::value_type & val)
{
  _max = std::max(_max, static_cast<OutType>(val));
}

template <typename InType, typename OutType>
void
Max<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.max(_max);
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Sum<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_sum = std::accumulate(data.begin(), data.end(), OutType());
  if (is_distributed)
    this->_communicator.sum(local_sum);
  return local_sum;
}

template <typename InType, typename OutType>
void
Sum<InType, OutType>::initialize()
{
  _sum = OutType();
}

template <typename InType, typename OutType>
void
Sum<InType, OutType>::update(const typename InType::value_type & val)
{
  _sum += val;
}

template <typename InType, typename OutType>
void
Sum<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.sum(_sum);
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
StdDev<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto count = data.size();
  auto sum = OutType();
  auto sum_of_square = OutType();
  for (const auto & val : data)
  {
    sum += val;
    sum_of_square += val * val;
  }
  if (is_distributed)
  {
    this->_communicator.sum(count);
    this->_communicator.sum(sum);
    this->_communicator.sum(sum_of_square);
  }
  return count <= 1 ? OutType() : std::sqrt((sum_of_square - sum * sum / count) / (count - 1));
}

template <typename InType, typename OutType>
void
StdDev<InType, OutType>::initialize()
{
  _count = 0;
  _sum = OutType();
  _sum_of_square = OutType();
}

template <typename InType, typename OutType>
void
StdDev<InType, OutType>::update(const typename InType::value_type & val)
{
  _count++;
  _sum += val;
  _sum_of_square += val * val;
}

template <typename InType, typename OutType>
void
StdDev<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.sum(_count);
    this->_communicator.sum(_sum);
    this->_communicator.sum(_sum_of_square);
  }
}

// STDERR //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
StdErr<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto count = data.size();
  if (is_distributed)
    this->_communicator.sum(count);
  return count == 0 ? OutType() : StdDev<InType, OutType>::compute(data, is_distributed) / std::sqrt(count);
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Ratio<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_max = data.empty() ? std::numeric_limits<OutType>::min()
                                : *std::max_element(data.begin(), data.end());
  auto local_min = data.empty() ? std::numeric_limits<OutType>::max()
                                : *std::min_element(data.begin(), data.end());
  if (is_distributed)
  {
    this->_communicator.min(local_min);
    this->_communicator.max(local_max);
  }
  return local_max / local_min;
}

template <typename InType, typename OutType>
void
Ratio<InType, OutType>::initialize()
{
  _min = std::numeric_limits<OutType>::max();
  _max = std::numeric_limits<OutType>::min();
}

template <typename InType, typename OutType>
void
Ratio<InType, OutType>::update(const typename InType::value_type & val)
{
  _min = std::min(_min, static_cast<OutType>(val));
  _max = std::max(_max, static_cast<OutType>(val));
}

template <typename InType, typename OutType>
void
Ratio<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    this->_communicator.min(_min);
    this->_communicator.max(_max);
  }
}

// L2NORM //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
L2Norm<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  auto local_sum = std::accumulate(
      data.begin(), data.end(), OutType(), [](OutType running_value, OutType current_value) {
        return running_value + current_value * current_value;
      });

  if (is_distributed)
    this->_communicator.sum(local_sum);

  return std::sqrt(local_sum);
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::initialize()
{
  _l2_norm = OutType();
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::update(const typename InType::value_type & val)
{
  _l2_norm += val * val;
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.sum(_l2_norm);
  _l2_norm = std::sqrt(_l2_norm);
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

#define createCalculators(InType, OutType)                                                         \
  template class Calculator<InType, OutType>;                                                      \
  template class Mean<InType, OutType>;                                                            \
  template class Max<InType, OutType>;                                                             \
  template class Min<InType, OutType>;                                                             \
  template class Sum<InType, OutType>;                                                             \
  template class StdDev<InType, OutType>;                                                          \
  template class StdErr<InType, OutType>;                                                          \
  template class Ratio<InType, OutType>;                                                           \
  template class L2Norm<InType, OutType>;                                                          \
  template std::unique_ptr<Calculator<InType, OutType>> makeCalculator<InType, OutType>(           \
      const MooseEnumItem &, const libMesh::ParallelObject &)

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
