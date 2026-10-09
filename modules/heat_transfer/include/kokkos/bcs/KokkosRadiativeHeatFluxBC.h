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
class KokkosRadiativeHeatFluxBC : public KokkosRadiativeHeatFluxBCBase
{
public:
  static InputParameters validParams();
  KokkosRadiativeHeatFluxBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real coefficient(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Emissivity of the boundary
  const Real _eps_boundary;

  /// View factor function
  const Moose::Kokkos::Function _view_factor_fn;

  /// Post-processor by which to scale boundary condition
  const Moose::Kokkos::PostprocessorValue _scale_pp;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosRadiativeHeatFluxBC::coefficient(const unsigned int qp, AssemblyDatum & datum) const
{
  return _scale_pp * _eps_boundary * _view_factor_fn.value(_t, datum.q_point(qp));
}
