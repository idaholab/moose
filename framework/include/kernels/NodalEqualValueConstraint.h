//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALEQUALVALUECONSTRAINT_H
#define NODALEQUALVALUECONSTRAINT_H

#include "NodalScalarKernel.h"

class NodalEqualValueConstraint;

template <>
InputParameters validParams<NodalEqualValueConstraint>();

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
  NodalEqualValueConstraint(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  std::vector<unsigned int> _val_number;
  std::vector<const VariableValue *> _value;
};

#endif /* NODALEQUALVALUECONSTRAINT_H */
