//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK.h"

/// St. Venant-Kirchhoff hyperelasticity
//
//    St. Venant-Kirchhoff hyperelasticity derivative from the
//    strain energy function W = lambda / 2 tr(E)^2 + mu tr(E^2)
//    with E = 1/2*(F^T*F - I)
//
class ComputeLagrangianStressStVenantKirchhoff : public ComputeLagrangianStressPK
{
public:
  static InputParameters validParams();
  ComputeLagrangianStressStVenantKirchhoff(const InputParameters & parameters);
  virtual ~ComputeLagrangianStressStVenantKirchhoff(){};

protected:
  /// Actual stress/Jacobian update
  virtual void computeQpPKStress();

protected:
  const Real _mu;
  const Real _lambda;
};
