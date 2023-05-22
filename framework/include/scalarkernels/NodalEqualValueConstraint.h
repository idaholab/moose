//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalScalarKernel.h"

/**
 * Constraint to enforce equal values (in 1D)
 *
 * This has to take 2 nodes and will enforce equality of values.
 * The form of the constraint is: r = value[0] - value[1] = 0.
 * This dictates the sign of the 'vg' parameter in the OneDEqualValueConstraintBC
 * class. It has a positive sign on the first node and negative on the other one.
 */
class NodalEqualValueConstraint : public NodalScalarKernel
{
public:
  static InputParameters validParams();

  NodalEqualValueConstraint(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  virtual Real computeQpResidual() override { mooseError("Unused"); }
  virtual Real computeQpJacobian() override { mooseError("Unused"); }
  const std::vector<unsigned int> _val_number;
  const std::vector<const VariableValue *> _value;
};
