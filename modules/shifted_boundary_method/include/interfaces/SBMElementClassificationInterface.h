//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "SBMUtils.h"
#include "libmesh/enum_order.h"

class MooseObject;

/**
 * Common element classification parameters for shifted boundary objects.
 */
class SBMElementClassificationInterface
{
public:
  static InputParameters validParams();
  explicit SBMElementClassificationInterface(const MooseObject * moose_object);
  virtual ~SBMElementClassificationInterface() = default;

protected:
  /// Threshold applied to the fraction outside the retained domain for a partial element
  const Real _lambda;

  /// Quadrature order used to estimate the retained-domain fraction
  const Order _qrule_order;

  /// Policy for assigning partial elements to a dedicated intercepted subdomain
  const SBMUtils::InterceptedSubdomainPolicy _intercepted_subdomain_policy;
};
