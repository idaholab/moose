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

#ifndef EQUALVALUENODALCONSTRAINT_H
#define EQUALVALUENODALCONSTRAINT_H

#include "NodalConstraint.h"

class EqualValueNodalConstraint;

template<>
InputParameters validParams<EqualValueNodalConstraint>();

class EqualValueNodalConstraint : public NodalConstraint
{
public:
  EqualValueNodalConstraint(const std::string & name, InputParameters parameters);
  virtual ~EqualValueNodalConstraint();

protected:
  virtual Real computeQpResidual(Moose::ConstraintType type);
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  Real _penalty;
};

#endif /* EQUALVALUENODALCONSTRAINT_H */
