//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"

/**
 * Declares one property per subdomain alongside one per quadrature point, to exercise a material
 * whose outputs have different storage granularity.
 *
 * The per-quadrature-point property is computed from the subdomain-constant one, which is only
 * possible because the coarser granularity is evaluated first.
 */
class KokkosMixedGranularityTest : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosMixedGranularityTest(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeSubdomainProperties(const unsigned int qp, Datum & datum) const
  {
    // Distinct per subdomain, so reading the wrong subdomain's slot is visible
    _coarse(datum, qp) = 10.0 * (datum.elem().subdomain + 1);
  }

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    const Real coarse = _coarse(datum, qp);

    // Offsetting by the quadrature point index makes the fine property vary within an element, and
    // reading the coarse property here is what requires the coarse pass to have run first
    _fine(datum, qp) = coarse + qp;
  }

private:
  /// Constant over each subdomain
  Moose::Kokkos::MaterialProperty<Real> _coarse;
  /// Varies per quadrature point
  Moose::Kokkos::MaterialProperty<Real> _fine;
};

/**
 * Declares a subdomain-constant property without defining computeSubdomainProperties(), so that the
 * framework's report of the missing hook can be tested.
 */
class KokkosMissingGranularityHookTest : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosMissingGranularityHookTest(const InputParameters & parameters);

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const
  {
    _fine(datum, qp) = qp;
  }

private:
  Moose::Kokkos::MaterialProperty<Real> _coarse;
  Moose::Kokkos::MaterialProperty<Real> _fine;
};
