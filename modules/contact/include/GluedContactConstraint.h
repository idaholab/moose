/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GLUEDCONTACTCONSTRAINT_H
#define GLUEDCONTACTCONSTRAINT_H

// MOOSE includes
#include "SparsityBasedContactConstraint.h"

#include "ContactMaster.h"

// Forward Declarations
class GluedContactConstraint;

template <>
InputParameters validParams<GluedContactConstraint>();

/**
 * A GluedContactConstraint forces the value of a variable to be the same on both sides of an
 * interface.
 */
class GluedContactConstraint : public SparsityBasedContactConstraint
{
public:
  GluedContactConstraint(const InputParameters & parameters);
  virtual ~GluedContactConstraint() {}

  virtual void timestepSetup();
  virtual void jacobianSetup();

  virtual void updateContactSet(bool beginning_of_step = false);

  virtual Real computeQpSlaveValue();

  virtual Real computeQpResidual(Moose::ConstraintType type);

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  /**
   * Compute off-diagonal Jacobian entries
   * @param type The type of coupling
   * @param jvar The index of the coupled variable
   */
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned int jvar);

  bool shouldApply();

protected:
  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;

  const Real _penalty;
  const Real _friction_coefficient;
  const Real _tension_release;
  bool _updateContactSet;

  NumericVector<Number> & _residual_copy;

  unsigned int _x_var;
  unsigned int _y_var;
  unsigned int _z_var;

  std::vector<unsigned int> _vars;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * _aux_solution;
};

#endif
