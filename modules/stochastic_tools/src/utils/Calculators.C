//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <numeric>

#include "Statistics.h"
#include "MooseEnumItem.h"
#include "MooseError.h"
#include "MooseRandom.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"

namespace StochasticTools
{

MultiMooseEnum
makeCalculatorEnum()
{
  return MultiMooseEnum("min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6 stderr=7");
}

// CALCULATOR //////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
Calculator<InType, OutType>::Calculator(const libMesh::ParallelObject & other,
                                        const std::string & name)
  : libMesh::ParallelObject(other), _name(name)
{
}

template <typename InType, typename OutType>
const std::string &
Calculator<InType, OutType>::name() const
{
  return _name;
}

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Mean<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_count, local_sum;
  local_count = data.size();
  local_sum = std::accumulate(data.begin(), data.end(), 0.);

  if (is_distributed)
  {
    this->_communicator.sum(local_count);
    this->_communicator.sum(local_sum);
  }

  return local_sum / local_count;
}

// MIN /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Min<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_min;
  local_min = *std::min_element(data.begin(), data.end());
  if (is_distributed)
    this->_communicator.min(local_min);
  return local_min;
}

// MAX /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Max<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_max;
  local_max = *std::max_element(data.begin(), data.end());
  if (is_distributed)
    this->_communicator.max(local_max);
  return local_max;
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Sum<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_sum = 0.;
  local_sum = std::accumulate(data.begin(), data.end(), 0.);
  if (is_distributed)
    this->_communicator.sum(local_sum);
  return local_sum;
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
StdDev<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real count = data.size();
  Real sum = std::accumulate(data.begin(), data.end(), 0.);

  if (is_distributed)
  {
    this->_communicator.sum(count);
    this->_communicator.sum(sum);
  }

  Real mean = sum / count;
  Real sum_of_squares = std::accumulate(
      data.begin(), data.end(), 0., [&mean](Real running_value, Real current_value) {
        return running_value + std::pow(current_value - mean, 2);
      });
  if (is_distributed)
    this->_communicator.sum(sum_of_squares);

  return std::sqrt(sum_of_squares / (count - 1.));
}

// STDERR //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
StdErr<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real count = data.size();
  if (is_distributed)
    this->_communicator.sum(count);
  return StdDev<InType, OutType>::compute(data, is_distributed) / std::sqrt(count);
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
Ratio<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_max = std::numeric_limits<Real>::min();
  Real local_min = std::numeric_limits<Real>::max();
  local_min = *std::min_element(data.begin(), data.end());
  local_max = *std::max_element(data.begin(), data.end());

  if (is_distributed)
  {
    this->_communicator.min(local_min);
    this->_communicator.max(local_max);
  }

  return local_min != 0. ? local_max / local_min : 0.;
}

// L2NORM //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
OutType
L2Norm<InType, OutType>::compute(const InType & data, bool is_distributed) const
{
  Real local_sum =
      std::accumulate(data.begin(), data.end(), 0., [](Real running_value, Real current_value) {
        return running_value + std::pow(current_value, 2);
      });

  if (is_distributed)
    this->_communicator.sum(local_sum);

  return std::sqrt(local_sum);
}

// makeCalculator //////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::unique_ptr<const Calculator<InType, OutType>>
makeCalculator(const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  if (item == "min")
    return libmesh_make_unique<const Min<InType, OutType>>(other, item);

  else if (item == "max")
    return libmesh_make_unique<const Max<InType, OutType>>(other, item);

  else if (item == "sum")
    return libmesh_make_unique<const Sum<InType, OutType>>(other, item);

  else if (item == "mean" || item == "average") // average is deprecated
    return libmesh_make_unique<const Mean<InType, OutType>>(other, item);

  else if (item == "stddev")
    return libmesh_make_unique<const StdDev<InType, OutType>>(other, item);

  else if (item == "stderr")
    return libmesh_make_unique<const StdErr<InType, OutType>>(other, item);

  else if (item == "norm2")
    return libmesh_make_unique<const L2Norm<InType, OutType>>(other, item);

  else if (item == "ratio")
    return libmesh_make_unique<const Ratio<InType, OutType>>(other, item);

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
  template std::unique_ptr<const Calculator<InType, OutType>> makeCalculator<InType, OutType>(     \
      const MooseEnumItem &, const libMesh::ParallelObject &)

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
