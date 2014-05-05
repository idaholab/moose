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

#ifndef MULTIDCONTACTCONSTRAINT_H
#define MULTIDCONTACTCONSTRAINT_H

//MOOSE includes
#include "NodeFaceConstraint.h"

#include "ContactMaster.h"

//Forward Declarations
class MultiDContactConstraint;

template<>
InputParameters validParams<MultiDContactConstraint>();

/**
 * A MultiDContactConstraint forces the value of a variable to be the same on both sides of an interface.
 */
class MultiDContactConstraint :
  public NodeFaceConstraint
{
public:
  MultiDContactConstraint(const std::string & name, InputParameters parameters);
  virtual ~MultiDContactConstraint(){}

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

  const unsigned int _component;

  const ContactModel _model;

  Real _penalty;

  unsigned int _x_var;
  unsigned int _y_var;
  unsigned int _z_var;

  const unsigned int _mesh_dimension;

  RealVectorValue _vars;
};

#endif
