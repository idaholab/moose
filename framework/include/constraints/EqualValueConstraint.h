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

#ifndef EQUALVALUECONSTRAINT_H
#define EQUALVALUECONSTRAINT_H

#include "FaceFaceConstraint.h"

class EqualValueConstraint;

template<>
InputParameters validParams<EqualValueConstraint>();

/**
 *
 */
class EqualValueConstraint : public FaceFaceConstraint
{
public:
  EqualValueConstraint(const std::string & name, InputParameters parameters);
  virtual ~EqualValueConstraint();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpResidualSide(Moose::ConstraintType res_type);
  virtual Real computeQpJacobianSide(Moose::ConstraintJacobianType jac_type);
};


#endif /* EQUALVALUECONSTRAINT_H */
