//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelValue.h"
#include "KokkosPolynomial.h"

/**
 * Reaction term (psi_i, exp(u) - p(v)), where p is a polynomial with coefficients given in the
 * input and v is a coupled variable. It applies every fix from Exercise 2.
 */
class KokkosPolynomialReaction : public Moose::Kokkos::KernelValue
{
public:
  static InputParameters validParams();

  KokkosPolynomialReaction(const InputParameters & parameters);

  // Snippet D: hooks are non-virtual templates on the final object type
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  // Snippet F: correctly spelled, so the Jacobian loop is registered
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

private:
  /// Coupled variable, bound by value (snippet A)
  const Moose::Kokkos::VariableValue _v;
  /// Polynomial coefficients in device-capable storage (snippet B)
  const Moose::Kokkos::Array<Real> _coefficients;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPolynomialReaction::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  // Snippet F: Kokkos math functions are callable on every backend
  return ::Kokkos::exp(_u(datum, qp)) - evaluatePolynomial(_coefficients, _v(datum, qp));
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosPolynomialReaction::precomputeQpJacobian(const unsigned int j,
                                               const unsigned int qp,
                                               AssemblyDatum & datum) const
{
  return ::Kokkos::exp(_u(datum, qp)) * _phi(datum, j, qp);
}
