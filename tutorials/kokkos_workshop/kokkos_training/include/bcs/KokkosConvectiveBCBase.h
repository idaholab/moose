//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCValue.h"

/**
 * Convective flux h(u) (u - u_inf), where the derived class provides h(u) through
 * computeCoefficient() and optionally its derivative through computeCoefficientDerivative().
 * This class is not registered; only its final derived classes are.
 */
class KokkosConvectiveBCBase : public Moose::Kokkos::IntegratedBCValue
{
public:
  static InputParameters validParams();

  KokkosConvectiveBCBase(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION Real precomputeQpJacobian(const unsigned int j,
                                            const unsigned int qp,
                                            AssemblyDatum & datum) const;

  /// Default for coefficients that do not depend on the solution; hidden by derived classes
  KOKKOS_FUNCTION Real computeCoefficientDerivative(const unsigned int /* qp */,
                                                    AssemblyDatum & /* datum */) const
  {
    return 0;
  }

protected:
  /// Far-field value
  const Real _u_inf;
};

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosConvectiveBCBase::precomputeQpResidual(const unsigned int qp, AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);
  return bc->computeCoefficient(qp, datum) * (_u(datum, qp) - _u_inf);
}

template <typename Derived>
KOKKOS_FUNCTION Real
KokkosConvectiveBCBase::precomputeQpJacobian(const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
{
  auto bc = static_cast<const Derived *>(this);
  return (bc->computeCoefficient(qp, datum) +
          bc->computeCoefficientDerivative(qp, datum) * (_u(datum, qp) - _u_inf)) *
         _phi(datum, j, qp);
}
