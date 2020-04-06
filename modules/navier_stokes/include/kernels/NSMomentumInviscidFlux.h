//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSKernel.h"

// ForwardDeclarations

/**
 * The inviscid flux (convective + pressure terms) for the
 * momentum conservation equations.
 */
class NSMomentumInviscidFlux : public NSKernel
{
public:
  static InputParameters validParams();

  NSMomentumInviscidFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variables
  const VariableValue & _pressure;

  // Parameters
  const unsigned int _component;

private:
  // To be used from both the on and off-diagonal
  // computeQpJacobian functions.  Variable numbering
  // should be in the canonical ordering regardless of
  // Moose's numbering.
  Real computeJacobianHelper(unsigned int m);
};
