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
