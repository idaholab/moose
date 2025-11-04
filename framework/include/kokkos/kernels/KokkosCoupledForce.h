//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernel.h"

/**
 * Implements a source term proportional to the value of a coupled variable. Weak form: $(\\psi_i,
 * -\\sigma v)$.
 */
class KokkosCoupledForce : public Moose::Kokkos::Kernel
{
public:
  static InputParameters validParams();

  KokkosCoupledForce(const InputParameters & parameters);

  KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
                                         const unsigned int qp,
                                         AssemblyDatum & datum) const;

private:
  /// Coupled variable number
  const unsigned int _v_var;
  /// Coupled variable
  const Moose::Kokkos::VariableValue _v;
  /// Multiplier for the coupled force term
  const Real _coef;
};

KOKKOS_FUNCTION inline Real
KokkosCoupledForce::computeQpResidual(const unsigned int i,
                                      const unsigned int qp,
                                      AssemblyDatum & datum) const
{
  return -_coef * _v(datum, qp) * _test(datum, i, qp);
}
