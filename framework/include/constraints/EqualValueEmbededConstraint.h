//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EqualValueEmbededConstraint_H
#define EqualValueEmbededConstraint_H

// MOOSE includes
#include "NodeElemConstraint.h"
// #include "ContactMaster.h"

/**
 * Formulations, currently only supports KINEMATIC and PENALTY
 */
enum Formulation
{
  KINEMATIC,
  PENALTY,
  INVALID
};

// Forward Declarations
class EqualValueEmbededConstraint;

template <>
InputParameters validParams<EqualValueEmbededConstraint>();

/**
 * A EqualValueEmbededConstraint forces the value of a variable to be the same
 * on overlapping portion of two blocks
 */
class EqualValueEmbededConstraint : public NodeElemConstraint
{
public:
  EqualValueEmbededConstraint(const InputParameters & parameters);

  virtual void timestepSetup() override;
  virtual void jacobianSetup() override;
  virtual void residualEnd() override;

  virtual Real computeQpSlaveValue() override;

  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override;

  /**
   * Compute off-diagonal Jacobian entries
   * @param jvar The index of the coupled variable
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  /**
   * Compute off-diagonal Jacobian entries
   * @param type The type of coupling
   * @param jvar The index of the coupled variable
   */
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                        unsigned int jvar) override;

  /**
   * Get the dof indices of the nodes connected to the slave node for a specific variable
   * @param var_num The number of the variable for which dof indices are gathered
   * @return bool indicating whether the coupled variable is one of the displacement variables
   */
  virtual void getConnectedDofIndices(unsigned int var_num) override;

  virtual bool addCouplingEntriesToJacobian() override { return true; }

  bool shouldApply() override;
  void computeReactionForce() override;

  static Formulation getFormulation(std::string name);

protected:
  MooseSharedPointer<DisplacedProblem> _displaced_problem;
  FEProblem & _fe_problem;

  const Formulation _formulation;

  const Real _penalty;

  NumericVector<Number> & _residual_copy;

  const unsigned int _mesh_dimension;
  Real _reaction_force;

  static Threads::spin_mutex _contact_set_mutex;
};

#endif
