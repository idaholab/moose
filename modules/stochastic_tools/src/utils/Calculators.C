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

std::unique_ptr<const Calculator>
makeCalculator(const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  if (item == "min")
    return libmesh_make_unique<const Min>(other);

  else if (item == "max")
    return libmesh_make_unique<const Max>(other);

  else if (item == "sum")
    return libmesh_make_unique<const Sum>(other);

  else if (item == "mean" || item == "average") // average is deprecated
    return libmesh_make_unique<const Mean>(other);

  else if (item == "stddev")
    return libmesh_make_unique<const StdDev>(other);

  else if (item == "stderr")
    return libmesh_make_unique<const StdErr>(other);

  else if (item == "norm2")
    return libmesh_make_unique<const L2Norm>(other);

  else if (item == "ratio")
    return libmesh_make_unique<const Ratio>(other);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

// CALCULATOR //////////////////////////////////////////////////////////////////////////////////////
Calculator::Calculator(const libMesh::ParallelObject & other) : libMesh::ParallelObject(other) {}

// MEAN ////////////////////////////////////////////////////////////////////////////////////////////
Mean::Mean(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
Mean::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_count, local_sum;
  local_count = data.size();
  local_sum = std::accumulate(data.begin(), data.end(), 0.);

  if (is_distributed)
  {
    _communicator.sum(local_count);
    _communicator.sum(local_sum);
  }

  return local_sum / local_count;
}

// MIN /////////////////////////////////////////////////////////////////////////////////////////////
Min::Min(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
Min::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_min;
  local_min = *std::min_element(data.begin(), data.end());
  if (is_distributed)
    _communicator.min(local_min);
  return local_min;
}

// MAX /////////////////////////////////////////////////////////////////////////////////////////////
Max::Max(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
Max::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_max;
  local_max = *std::max_element(data.begin(), data.end());
  if (is_distributed)
    _communicator.max(local_max);
  return local_max;
}

// SUM /////////////////////////////////////////////////////////////////////////////////////////////
Sum::Sum(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
Sum::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_sum = 0.;
  local_sum = std::accumulate(data.begin(), data.end(), 0.);
  if (is_distributed)
    _communicator.sum(local_sum);
  return local_sum;
}

// STDDEV //////////////////////////////////////////////////////////////////////////////////////////
StdDev::StdDev(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
StdDev::compute(const std::vector<Real> & data, bool is_distributed) const
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

  return std::sqrt(sum_of_squares / (count - 1.));
}

// STDERR //////////////////////////////////////////////////////////////////////////////////////////
StdErr::StdErr(const libMesh::ParallelObject & other) : StdDev(other) {}

Real
StdErr::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real count = data.size();
  if (is_distributed)
    _communicator.sum(count);
  return StdDev::compute(data, is_distributed) / std::sqrt(count);
}

// RATIO ///////////////////////////////////////////////////////////////////////////////////////////
Ratio::Ratio(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
Ratio::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_max = std::numeric_limits<Real>::min();
  Real local_min = std::numeric_limits<Real>::max();
  local_min = *std::min_element(data.begin(), data.end());
  local_max = *std::max_element(data.begin(), data.end());

  if (is_distributed)
  {
    _communicator.min(local_min);
    _communicator.max(local_max);
  }

  return local_min != 0. ? local_max / local_min : 0.;
}

// L2NORM //////////////////////////////////////////////////////////////////////////////////////////
L2Norm::L2Norm(const libMesh::ParallelObject & other) : Calculator(other) {}

Real
L2Norm::compute(const std::vector<Real> & data, bool is_distributed) const
{
  Real local_sum =
      std::accumulate(data.begin(), data.end(), 0., [](Real running_value, Real current_value) {
        return running_value + std::pow(current_value, 2);
      });

  if (is_distributed)
    _communicator.sum(local_sum);

  return std::sqrt(local_sum);
}
} // Statistics namespace
