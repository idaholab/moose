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
#include "MooseTypes.h"
#include "MooseEnumItem.h"
#include "MooseError.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"

namespace Statistics
{

MultiMooseEnum
makeCalculatorEnum()
{
  return MultiMooseEnum("min=0 max=1 sum=2 mean=3 stddev=4 norm2=5 ratio=6");
}

std::unique_ptr<Calculator>
makeCalculator(const MooseEnumItem & item, const MooseObject & other)
{
  if (item == "min")
    return libmesh_make_unique<Min>(other);

  else if (item == "max")
    return libmesh_make_unique<Max>(other);

  else if (item == "sum")
    return libmesh_make_unique<Sum>(other);

  else if (item == "mean" || item == "average") // average is deprecated
    return libmesh_make_unique<Mean>(other);

  else if (item == "stddev")
    return libmesh_make_unique<StdDev>(other);

  else if (item == "norm2")
    return libmesh_make_unique<L2Norm>(other);

  else if (item == "ratio")
    return libmesh_make_unique<Ratio>(other);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

// CALCULATOR //////////////////////////////////////////////////////////////////////////////////////
Calculator::Calculator(const MooseObject & other) : libMesh::ParallelObject(other) {}

Real
Calculator::compute(const std::vector<Real> & data)
{
  initialize(false);
  execute(data, false);
  finalize(false);
  return value();
}

Mean::Mean(const MooseObject & other) : Calculator(other) {}

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
void
Mean::initialize(bool /*is_distributed*/)
{
  _local_count = 0;
  _local_sum = 0;
}

void
Mean::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_count = data.size();
  _local_sum = std::accumulate(data.begin(), data.end(), 0.);
}

void
Mean::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    _communicator.sum(_local_count);
    _communicator.sum(_local_sum);
  }
}

Real
Mean::value()
{
  return _local_sum / _local_count;
}

// MIN /////////////////////////////////////////////////////////////////////////////////////////////
Min::Min(const MooseObject & other) : Calculator(other) {}

void
Min::initialize(bool /*is_distributed*/)
{
  _local_min = std::numeric_limits<Real>::max();
}

void
Min::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_min = *std::min_element(data.begin(), data.end());
}

void
Min::finalize(bool is_distributed)
{
  if (is_distributed)
    _communicator.min(_local_min);
}

Real
Min::value()
{
  return _local_min;
}

// MAX /////////////////////////////////////////////////////////////////////////////////////////////
Max::Max(const MooseObject & other) : Calculator(other) {}

void
Max::initialize(bool /*is_distributed*/)
{
  _local_max = std::numeric_limits<Real>::min();
}

void
Max::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_max = *std::max_element(data.begin(), data.end());
}

void
Max::finalize(bool is_distributed)
{
  if (is_distributed)
    _communicator.max(_local_max);
}

Real
Max::value()
{
  return _local_max;
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
Sum::Sum(const MooseObject & other) : Calculator(other) {}

void
Sum::initialize(bool /*is_distributed*/)
{
  _local_sum = 0.;
}

void
Sum::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_sum = std::accumulate(data.begin(), data.end(), 0.);
}

void
Sum::finalize(bool is_distributed)
{
  if (is_distributed)
    _communicator.sum(_local_sum);
}

Real
Sum::value()
{
  return _local_sum;
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
StdDev::StdDev(const MooseObject & other) : Calculator(other) {}

void
StdDev::execute(const std::vector<Real> & data, bool is_distributed)
{
  Real count = data.size();
  Real sum = std::accumulate(data.begin(), data.end(), 0.);

  if (is_distributed)
  {
    _communicator.sum(count);
    _communicator.sum(sum);
  }

  Real mean = sum / count;
  Real sum_of_squares = std::accumulate(
      data.begin(), data.end(), 0., [&mean](Real running_value, Real current_value) {
        return running_value + std::pow(current_value - mean, 2);
      });
  if (is_distributed)
    _communicator.sum(sum_of_squares);

  _value = std::sqrt(sum_of_squares / (count - 1.));
}

Real
StdDev::value()
{
  return _value;
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
Ratio::Ratio(const MooseObject & other) : Calculator(other) {}

void
Ratio::initialize(bool /*is_distributed*/)
{
  _local_max = std::numeric_limits<Real>::min();
  _local_min = std::numeric_limits<Real>::max();
}

void
Ratio::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_min = *std::min_element(data.begin(), data.end());
  _local_max = *std::max_element(data.begin(), data.end());
}

void
Ratio::finalize(bool is_distributed)
{
  if (is_distributed)
  {
    _communicator.min(_local_min);
    _communicator.max(_local_max);
  }
}

Real
Ratio::value()
{
  return _local_min != 0. ? _local_max / _local_min : 0.;
}

// L2NORM //////////////////////////////////////////////////////////////////////////////////////////
L2Norm::L2Norm(const MooseObject & other) : Calculator(other) {}

void
L2Norm::initialize(bool /*is_distributed*/)
{
  _local_sum = 0;
}

void
L2Norm::execute(const std::vector<Real> & data, bool /*is_distributed*/)
{
  _local_sum =
      std::accumulate(data.begin(), data.end(), 0., [](Real running_value, Real current_value) {
        return running_value + std::pow(current_value, 2);
      });
}

void
L2Norm::finalize(bool is_distributed)
{
  if (is_distributed)
    _communicator.sum(_local_sum);
}

Real
L2Norm::value()
{
  return std::sqrt(_local_sum);
}

} // Statistics namespace
