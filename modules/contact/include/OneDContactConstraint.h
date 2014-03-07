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

#ifndef TIEDVALUECONSTRAINT_H
#define TIEDVALUECONSTRAINT_H

//MOOSE includes
#include "NodeFaceConstraint.h"

//Forward Declarations
class OneDContactConstraint;

template<>
InputParameters validParams<OneDContactConstraint>();

/**
 * A OneDContactConstraint forces the value of a variable to be the same on both sides of an interface.
 */
class OneDContactConstraint :
  public NodeFaceConstraint
{
public:
  OneDContactConstraint(const std::string & name, InputParameters parameters);
  virtual ~OneDContactConstraint(){}

  virtual void timestepSetup();
  virtual void jacobianSetup();

  virtual void updateContactSet();

  virtual Real computeQpSlaveValue();

  virtual Real computeQpResidual(Moose::ConstraintType type);

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  bool shouldApply();
protected:
  NumericVector<Number> & _residual_copy;

  bool _jacobian_update;
};

#endif
