//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NodeFaceConstraint.h"
#include "PenetrationLocator.h"
#include "Coupleable.h"

// Forward Declarations
enum class ExplicitDynamicsContactModel;

/**
 * Method to test overwriting of solution variables from NonlinearSystemBase.
 */
class ExplicitDynamicsOverwrite : public NodeFaceConstraint
{
public:
  static InputParameters validParams();

  ExplicitDynamicsOverwrite(const InputParameters & parameters);

  virtual void timestepSetup() override;
  virtual void jacobianSetup() override {}
  virtual void residualEnd() override {}

  virtual Real computeQpSecondaryValue() override;
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override {}

  /**
   * Compute off-diagonal Jacobian entries
   * @param jvar The index of the coupled variable
   */
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/) override {}

  virtual Real computeQpJacobian(Moose::ConstraintJacobianType /*type*/) override { return 0.0; }

  /**
   * Compute off-diagonal Jacobian entries
   * @param type The type of coupling
   * @param jvar The index of the coupled variable
   */
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/) override
  {
    return 0.0;
  }

  bool shouldApply() override;
  void computeContactForce(const Node & node, PenetrationInfo * pinfo);
  virtual bool isExplicitConstraint() const override { return true; }
  /**
   * Return false so that the nonlinear system does not try to add Jacobian entries
   * from the contact forces.
   * @return bool indicating whether we need to couple Jacobian entries
   */
  virtual bool addCouplingEntriesToJacobian() override { return false; }

  virtual void overwriteBoundaryVariables(NumericVector<Number> & soln,
                                          const Node & secondary_node) const override;

protected:
  Real gapOffset(const Node & node);

  const unsigned int _component;

  const unsigned int _mesh_dimension;

  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;

  const bool _has_secondary_gap_offset;
  const MooseVariable * const _secondary_gap_offset_var;
  const bool _has_mapped_primary_gap_offset;
  const MooseVariable * const _mapped_primary_gap_offset_var;

  /// Whether to overwrite contact boundary nodal solution
  const bool _overwrite_current_solution;

  /// Nodal gap rate (exercise writable variables from framework)
  MooseWritableVariable * _gap_rate;

private:
  std::unordered_map<dof_id_type, Real> _dof_to_gap;
};
