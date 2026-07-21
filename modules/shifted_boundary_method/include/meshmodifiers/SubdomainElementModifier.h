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
#include "PointInSubdomainCheckUO.h" // UO with subdomain geometry checks
#include "Function.h"
#include "libmesh/quadrature_gauss.h"

class SubdomainElementModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();
  SubdomainElementModifier(const InputParameters & parameters);

protected:
  virtual SubdomainID computeSubdomainID() override;

  /// Convert integer to Order
  Order intToOrder(int value);

  /// The user object containing subdomain-wise in/out checkers
  const PointInSubdomainCheckUO & _subdomain_id_tester;

  /// The geometric threshold to decide between inside/outside
  Real _lambda;

  /// @brief  Quadrature order used for active‑area estimation
  int _qrule_order;

  /// @brief If true, mark the intercepted elements with the subdomain ID
  bool _mark_intercepted;

  /// @brief The subdomain ID to assign to intercepted elements
  SubdomainID _subdomain_id_intercepted;
};
