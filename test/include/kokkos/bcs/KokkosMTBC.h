//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBC.h"

class KokkosMTBC : public Moose::Kokkos::IntegratedBC
{
public:
  static InputParameters validParams();

  KokkosMTBC(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  /**
   * Value of grad(u) on the boundary.
   */
  const Real _value;
  const Moose::Kokkos::MaterialProperty<Real> _mat;
};

KOKKOS_FUNCTION inline Real
KokkosMTBC::computeQpResidual(const unsigned int i,
                              const unsigned int qp,
                              AssemblyDatum & datum) const
{
  return -_test(datum, i, qp) * _value * _mat(datum, qp);
}
