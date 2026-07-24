//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "libmesh/enum_order.h"

/**
 * Common partial-element classification parameters for shifted boundary element modifiers.
 */
class SBMElementSubdomainModifierBase : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();
  SBMElementSubdomainModifierBase(const InputParameters & parameters);

protected:
  /**
   * Return whether a partial element is inactive, i.e. its inactive fraction exceeds lambda.
   *
   * The endpoint handling preserves the convention that lambda zero rejects and lambda one
   * accepts a partially active element.
   */
  static bool isInactive(Real active_fraction, Real lambda);

  /// Threshold applied to the inactive fraction of a partially active element
  const Real _lambda;

  /// Quadrature order used to estimate the active fraction
  const Order _qrule_order;

  /// Whether to assign a dedicated subdomain ID to intercepted elements
  const bool _mark_intercepted;

  /// Subdomain ID assigned to intercepted elements
  const SubdomainID _subdomain_id_intercepted;
};
