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

registerMooseCalculator(0, "min", Statistics::Min);
registerMooseCalculator(1, "max", Statistics::Max);
registerMooseCalculator(2, "sum", Statistics::Sum);
registerMooseCalculator(3, "mean", Statistics::Mean);
registerMooseCalculator(4, "stddev", Statistics::StdDev);
registerMooseCalculator(5, "norm2", Statistics::L2Norm);
registerMooseCalculator(6, "ratio", Statistics::Ratio);
registerMooseCalculator(7, "stderr", Statistics::StdErr);

namespace Statistics
{
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
