//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADArrayIntegratedBC.h"

/**
 * Robin boundary condition for all components of an array variable,
 * grad(u) . n = h - alpha u, with automatic differentiation
 */
class ADArrayRobinBC : public ADArrayIntegratedBC
{
public:
  static InputParameters validParams();

  ADArrayRobinBC(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(ADRealEigenVector & residual) override;

  /// prescribed flux h
  const RealEigenVector _value;
  /// Robin coefficient alpha
  const RealEigenVector _alpha;
};
