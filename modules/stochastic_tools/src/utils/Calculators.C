//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Calculators.h"

namespace StochasticTools
{

MultiMooseEnum
makeCalculatorEnum()
{
  return MultiMooseEnum(
      "min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6 stderr=7 median=8 meanabs=9");
}

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
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
  _sum += static_cast<OutType>(val);
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
  if (_count > 0)
    _sum /= static_cast<OutType>(_count);
}

// MEAN ABS ////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
MeanAbsoluteValue<InType, OutType>::update(const typename InType::value_type & val)
{
  Mean<InType, OutType>::update(std::abs(val));
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
Sum<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.sum(this->_sum);
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
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
  _sum += static_cast<OutType>(val);
  _sum_of_square += MathUtils::pow(static_cast<OutType>(val), 2);
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

  if (_count <= 1)
    _sum_of_square = 0;
  else
    _sum_of_square = std::sqrt(std::abs(_sum_of_square - _sum * _sum / _count) / (_count - 1));
}

// STDERR //////////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
void
StdErr<InType, OutType>::finalize(bool is_distributed)
{
  StdDev<InType, OutType>::finalize(is_distributed);
  this->_sum_of_square /= std::sqrt(this->_count);
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
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
  if (_min > val)
    _min = static_cast<OutType>(val);
  if (_max < val)
    _max = static_cast<OutType>(val);
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
void
L2Norm<InType, OutType>::initialize()
{
  _l2_norm = OutType();
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::update(const typename InType::value_type & val)
{
  _l2_norm += MathUtils::pow(static_cast<OutType>(val), 2);
}

template <typename InType, typename OutType>
void
L2Norm<InType, OutType>::finalize(bool is_distributed)
{
  if (is_distributed)
    this->_communicator.sum(_l2_norm);
  _l2_norm = std::sqrt(_l2_norm);
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
  _storage.push_back(static_cast<OutType>(val));
}

template <typename InType, typename OutType>
void
Median<InType, OutType>::finalize(bool is_distributed)
{
  // Make sure we aren't doing anything silly like taking the median of an empty vector
  _median = OutType();
  auto count = _storage.size();
  if (is_distributed)
    this->_communicator.sum(count);
  if (count == 0)
    return;

  if (!is_distributed || this->n_processors() == 1)
  {
    std::sort(_storage.begin(), _storage.end());
    if (count % 2)
      _median = _storage[count / 2];
    else
      _median += (_storage[count / 2] + _storage[count / 2 - 1]) / 2;
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
    for (const auto & i : index_range(sz))
      if (sz[i])
      {
        if (this->processor_id() == i)
          _median = _storage[0];
        this->_communicator.broadcast(_median, i);
        break;
      }

    // Count number of values greater than, less than, and equal to _median
    std::vector<dof_id_type> m(3, 0);
    for (const auto & val : _storage)
    {
      if (_median < val)
        m[0]++;
      else if (val < _median)
        m[1]++;
    }
    this->_communicator.sum(m);
    m[2] = n - m[0] - m[1];

    // Remove greater than equal to
    if ((m[0] + m[2]) <= kgt)
    {
      _storage.erase(std::remove_if(_storage.begin(),
                                    _storage.end(),
                                    [this](const OutType & val) { return val >= _median; }),
                     _storage.end());
      kgt -= m[0] + m[2];
    }
    // Remove less than equal to
    else if ((m[1] + m[2]) <= klt)
    {
      _storage.erase(std::remove_if(_storage.begin(),
                                    _storage.end(),
                                    [this](const OutType & val) { return val <= _median; }),
                     _storage.end());
      klt -= m[1] + m[2];
    }
    // If the number of points is odd, then we've found it
    else if (count % 2)
      break;
    // Get average of the two middle numbers
    else
    {
      OutType num2;
      // Find the next greater than
      if (m[0] > kgt)
      {
        num2 = std::numeric_limits<OutType>::max();
        for (const auto & val : _storage)
          if (_median < val && val < num2)
            num2 = val;
        this->_communicator.min(num2);
      }
      // Find the next less than
      else if (m[1] > klt)
      {
        num2 = std::numeric_limits<OutType>::min();
        for (const auto & val : _storage)
          if (val < _median && num2 < val)
            num2 += val;
        this->_communicator.max(num2);
      }
      // Otherwise we know the other number is equal
      else
        num2 = _median;

      _median = (_median + num2) / 2;
      break;
    }
  }
}

// CalculatorBuilder
// //////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::unique_ptr<Calculator<InType, OutType>>
CalculatorBuilder<InType, OutType>::build(const MooseEnumItem & item,
                                          const libMesh::ParallelObject & other)
{
  if (item == "min")
    return std::make_unique<Min<InType, OutType>>(other, item);

  else if (item == "max")
    return std::make_unique<Max<InType, OutType>>(other, item);

  else if (item == "sum")
    return std::make_unique<Sum<InType, OutType>>(other, item);

  else if (item == "mean" || item == "average") // average is deprecated
    return std::make_unique<Mean<InType, OutType>>(other, item);

  else if (item == "stddev")
    return std::make_unique<StdDev<InType, OutType>>(other, item);

  else if (item == "stderr")
    return std::make_unique<StdErr<InType, OutType>>(other, item);

  else if (item == "norm2")
    return std::make_unique<L2Norm<InType, OutType>>(other, item);

  else if (item == "ratio")
    return std::make_unique<Ratio<InType, OutType>>(other, item);

  else if (item == "median")
    return std::make_unique<Median<InType, OutType>>(other, item);

  else if (item == "meanabs")
    return std::make_unique<MeanAbsoluteValue<InType, OutType>>(other, item);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

#define createCalculators(InType, OutType)                                                         \
  template class Mean<InType, OutType>;                                                            \
  template class Max<InType, OutType>;                                                             \
  template class Min<InType, OutType>;                                                             \
  template class Sum<InType, OutType>;                                                             \
  template class StdDev<InType, OutType>;                                                          \
  template class StdErr<InType, OutType>;                                                          \
  template class Ratio<InType, OutType>;                                                           \
  template class L2Norm<InType, OutType>;                                                          \
  template class Median<InType, OutType>;                                                          \
  template struct CalculatorBuilder<InType, OutType>

createCalculators(std::vector<Real>, Real);
createCalculators(std::vector<int>, Real);

} // StocasticTools namespace
