//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalReporter.h"

class KokkosNodalStatistics : public Moose::Kokkos::NodalReporter
{
public:
  static InputParameters validParams();

  KokkosNodalStatistics(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void executeShim(const Derived & reducer, Datum & datum, Real * result) const;

  KOKKOS_FUNCTION void join(ReducerLoop, Real * result, const Real * source) const;
  KOKKOS_FUNCTION void init(ReducerLoop, Real * result) const;

protected:
  virtual void initialize() override;
  virtual void finalize() override;

private:
  const std::string _base_name;
  Real & _max;
  Real & _min;
  Real & _average;
  unsigned int & _number_nodes;
};

template <typename Derived>
KOKKOS_FUNCTION inline void
KokkosNodalStatistics::executeShim(const Derived & reducer, Datum & datum, Real * result) const
{
  // Get value to to update statistics
  Real value = reducer.computeValue(datum);

  if (result[0] < value)
    result[0] = value;

  if (result[1] > value)
    result[1] = value;

  // Update the total and the number to get the average when "finalizing"
  result[2] += value;
  result[3]++;
}

KOKKOS_FUNCTION inline void
KokkosNodalStatistics::join(ReducerLoop, Real * result, const Real * source) const
{
  result[0] = Kokkos::max(result[0], source[0]);
  result[1] = Kokkos::min(result[1], source[1]);
  result[2] += source[2];
  result[3] += source[3];
}

KOKKOS_FUNCTION inline void
KokkosNodalStatistics::init(ReducerLoop, Real * result) const
{
  result[0] = Kokkos::Experimental::finite_min_v<Real>;
  result[1] = Kokkos::Experimental::finite_max_v<Real>;
  result[2] = 0;
  result[3] = 0;
}
