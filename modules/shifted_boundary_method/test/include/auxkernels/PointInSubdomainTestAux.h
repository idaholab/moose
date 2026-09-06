//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class PointInSubdomainCheckUO;

/**
 * Test-only AuxKernel that exposes the PointInSubdomainCheckUO point-query
 * accessors (whichSubdomain, ifInside) so coverage tests can exercise them
 * without adding production callers.
 */
class PointInSubdomainTestAux : public AuxKernel
{
public:
  static InputParameters validParams();

  explicit PointInSubdomainTestAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// The in-out test whose per-subdomain point queries are exercised.
  const PointInSubdomainCheckUO & _subdomain_checker;
  /// Which accessor to call: "which_subdomain" or "if_inside".
  const MooseEnum _method;
};
