//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAuxKernel.h"

class KokkosMaterialRealAux : public Moose::Kokkos::AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for this object
   */
  KokkosMaterialRealAux(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeValue(const unsigned int qp, ResidualDatum & datum) const;

private:
  /// Material property for this AuxKernel
  const Moose::Kokkos::MaterialProperty<Real> _prop;

  /// Multiplier for the material property
  const Real _factor;

  /// Value to be added to the material property
  const Real _offset;
};

KOKKOS_FUNCTION inline Real
KokkosMaterialRealAux::computeValue(const unsigned int qp, ResidualDatum & datum) const
{
  return _prop(datum, qp) * _factor + _offset;
}
