//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosRadiativeHeatFluxBCBase.h"

/**
 * Radiative heat transfer boundary condition for a plate heat structure
 */
class KokkosRadiativeHeatFluxBC final
  : public KokkosRadiativeHeatFluxBCBase<KokkosRadiativeHeatFluxBC>
{
public:
  static InputParameters validParams();
  KokkosRadiativeHeatFluxBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real coefficient() const;

private:
  /// Post-processor by which to scale boundary condition
  const Moose::Kokkos::PostprocessorValue _scale_pp;
};

KOKKOS_FUNCTION inline Real
KokkosRadiativeHeatFluxBC::coefficient() const
{
  return _scale_pp * _eps_boundary;
}
