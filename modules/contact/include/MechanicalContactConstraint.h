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

#ifndef MECHANICALCONTACTCONSTRAINT_H
#define MECHANICALCONTACTCONSTRAINT_H

//MOOSE includes
#include "NodeFaceConstraint.h"

#include "ContactMaster.h"

//Forward Declarations
class MechanicalContactConstraint;

template<>
InputParameters validParams<MechanicalContactConstraint>();

/**
 * A MechanicalContactConstraint forces the value of a variable to be the same on both sides of an interface.
 */
class MechanicalContactConstraint :
  public NodeFaceConstraint
{
public:
  MechanicalContactConstraint(const std::string & name, InputParameters parameters);
  virtual ~MechanicalContactConstraint(){}

  virtual void timestepSetup();
  virtual void jacobianSetup();

  virtual void updateContactSet(bool beginning_of_step = false);

  virtual Real computeQpSlaveValue();

  virtual Real computeQpResidual(Moose::ConstraintType type);

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  virtual bool addCouplingEntriesToJacobian() { return _master_slave_jacobian; }

  bool shouldApply();
  void computeContactForce(PenetrationInfo * pinfo);

protected:

  Real nodalArea(PenetrationInfo & pinfo);
  Real getPenalty(PenetrationInfo & pinfo);

  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;
  const bool _normalize_penalty;

  const Real _penalty;
  const Real _friction_coefficient;
  const Real _tension_release;
  bool _update_contact_set;
  Real _time_last_called;

  NumericVector<Number> & _residual_copy;
//  std::map<Point, PenetrationInfo *> _point_to_info;

  unsigned int _x_var;
  unsigned int _y_var;
  unsigned int _z_var;

  const unsigned int _mesh_dimension;

  RealVectorValue _vars;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * _aux_solution;

  const bool _master_slave_jacobian;
};

#endif
