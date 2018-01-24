/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
