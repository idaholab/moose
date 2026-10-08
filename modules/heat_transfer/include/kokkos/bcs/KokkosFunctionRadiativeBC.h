//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosRadiativeHeatFluxBCBase.h"

/**
 * Kokkos boundary condition for radiative heat exchange where the emissivity is given by a
 * function
 */
class KokkosFunctionRadiativeBC : public KokkosRadiativeHeatFluxBCBase
{
public:
  static InputParameters validParams();

  KokkosFunctionRadiativeBC(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real coefficient(const unsigned int qp, AssemblyDatum & datum) const;

private:
  /// Emissivity function
  const Moose::Kokkos::Function _emissivity;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosFunctionRadiativeBC::coefficient(const unsigned int qp, AssemblyDatum & datum) const
{
  return _emissivity.value(_t, datum.q_point(qp));
}
