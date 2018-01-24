//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EQUALGRADIENTCONSTRAINT_H
#define EQUALGRADIENTCONSTRAINT_H

#include "FaceFaceConstraint.h"

class EqualGradientConstraint;

template <>
InputParameters validParams<EqualGradientConstraint>();

/**
 * Constrain a specified component of the gradient of a variable to be the same
 * on both sides of an interface.
 */
class EqualGradientConstraint : public FaceFaceConstraint
{
public:
  EqualGradientConstraint(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpResidualSide(Moose::ConstraintType res_type) override;
  virtual Real computeQpJacobianSide(Moose::ConstraintJacobianType jac_type) override;

  const unsigned int _component;
};

#endif /* EQUALGRADIENTCONSTRAINT_H */
