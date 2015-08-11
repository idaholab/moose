/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAPCONDUCTANCECONSTRAINT_H
#define GAPCONDUCTANCECONSTRAINT_H

#include "FaceFaceConstraint.h"

class GapConductanceConstraint;

template<>
InputParameters validParams<GapConductanceConstraint>();

/**
 *
 */
class GapConductanceConstraint : public FaceFaceConstraint
{
public:
  GapConductanceConstraint(const InputParameters & parameters);
  virtual ~GapConductanceConstraint();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpResidualSide(Moose::ConstraintType res_type);
  virtual Real computeQpJacobian();
  virtual Real computeQpJacobianSide(Moose::ConstraintJacobianType jac_type);

  Real distance(const Point & a, const Point & b);

  Real _k;
};


#endif /* GAPCONDUCTANCECONSTRAINT_H */
