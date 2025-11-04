//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDiffusion.h"

class KokkosHeatConduction : public KokkosDiffusion
{
public:
  static InputParameters validParams();

  KokkosHeatConduction(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int i,
                                         const unsigned int j,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient;
  Moose::Kokkos::MaterialProperty<Real> _diffusion_coefficient_dT;
};

KOKKOS_FUNCTION inline Real
KokkosHeatConduction::computeQpResidual(const unsigned int i,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  return _diffusion_coefficient(datum, qp) * KokkosDiffusion::computeQpResidual(i, qp, datum);
}

KOKKOS_FUNCTION inline Real
KokkosHeatConduction::computeQpJacobian(const unsigned int i,
                                        const unsigned int j,
                                        const unsigned int qp,
                                        AssemblyDatum & datum) const
{
  Real jac =
      _diffusion_coefficient(datum, qp) * KokkosDiffusion::computeQpJacobian(i, j, qp, datum);
  if (_diffusion_coefficient_dT)
    jac += _diffusion_coefficient_dT(datum, qp) * _phi(datum, j, qp) *
           KokkosDiffusion::computeQpResidual(i, qp, datum);
  return jac;
}
