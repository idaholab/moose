//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAverageValue.h"

/**
 * Kokkos total volume fraction of a coupled mineral species
 */
class KokkosTotalMineralVolumeFraction : public KokkosElementAverageValue
{
public:
  static InputParameters validParams();

  KokkosTotalMineralVolumeFraction(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpIntegral(const unsigned int qp, Datum & datum) const
  {
    return _molar_volume * _u(datum, qp);
  }

private:
  /// Molar volume of the coupled mineral species
  const Real _molar_volume;
};
