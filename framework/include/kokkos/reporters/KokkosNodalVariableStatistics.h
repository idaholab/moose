//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalStatistics.h"

class KokkosNodalVariableStatistics : public KokkosNodalStatistics
{
public:
  static InputParameters validParams();

  KokkosNodalVariableStatistics(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(Datum & datum) const;

protected:
  /// The coupled variable used.
  const Moose::Kokkos::VariableValue _v;
};

KOKKOS_FUNCTION inline Real
KokkosNodalVariableStatistics::computeValue(Datum & datum) const
{
  return _v(datum, 0);
}
