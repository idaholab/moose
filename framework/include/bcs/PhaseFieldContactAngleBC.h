//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Implements a simple constant Neumann BC where grad(u)=alpha * v on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class PhaseFieldContactAngleBC : public ADIntegratedBC
{
public:
  PhaseFieldContactAngleBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// reference to coupled variable, pf here
  const ADVariableValue & _pf; 
  /// Gradient of coupled variable
  const ADVariableGradient & _grad_pf;
  /// Interface width
  const Real & _epsilon;
  /// Mixing energy density
  const Real & _lambda;
  /// Surface tension coefficient
  const Real & _sigma;
  /// Contact angle of the fluid with the wall boundary in Radians
  const Real & _contactangle;

};
