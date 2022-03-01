//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraint.h"

/**
 * Constrain the value of a variable to be the same on both sides of an
 * interface.
 */
class OldEqualValueConstraint : public MortarConstraint
{
public:
  static InputParameters validParams();

  OldEqualValueConstraint(const InputParameters & parameters);

protected:
  Real computeQpResidual(Moose::MortarType mortar_type) final;
  Real computeQpJacobian(Moose::ConstraintJacobianType jacobian_type, unsigned int jvar) final;
};
