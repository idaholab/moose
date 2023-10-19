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
#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
enum class ExplicitDynamicsContactModel;

/**
 * A ExplicitDynamicsContactConstraint does mechanical contact for explicit dynamics simulations.
 */
class ExplicitDynamicsContactConstraint : public NodeFaceConstraint,
                                          public TwoMaterialPropertyInterface
{
public:
  static InputParameters validParams();

  ExplicitDynamicsContactConstraint(const InputParameters & parameters);

  virtual void timestepSetup() override;
  virtual void jacobianSetup() override {}
  virtual void residualEnd() override;

  virtual void updateContactStatefulData(bool beginning_of_step);
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

  /**
   * Determine whether the coupled variable is one of the displacement variables,
   * and find its component
   * @param var_num The number of the variable to be checked
   * @param component The component index computed in this routine
   * @return bool indicating whether the coupled variable is one of the displacement variables
   */
  bool getCoupledVarComponent(unsigned int var_num, unsigned int & component);

  bool shouldApply() override;
  void computeContactForce(const Node & node, PenetrationInfo * pinfo, bool update_contact_set);

  /**
   * Return false so that the nonlinear system does not try to add Jacobian entries
   * from the contact forces.
   * @return bool indicating whether we need to couple Jacobian entries
   */
  virtual bool addCouplingEntriesToJacobian() override { return false; }

protected:
  /**
   * Determine "Lagrange multipliers" from the iterative solution of the impact problem.
   * @param node The number of the variable to be checked
   * @param pinfo The component index computed in this routine
   */
  void solveImpactEquations(const Node & node, PenetrationInfo * pinfo);

  const std::set<BoundaryID> getBoundaryIDs() const
  {
    std::set<BoundaryID> result;
    result.insert(_primary);
    result.insert(_secondary);

    return result;
  }

  MooseSharedPointer<DisplacedProblem> _displaced_problem;
  Real gapOffset(const Node & node);
  Real nodalArea(const Node & node);
  Real getPenalty(const Node & node);

  const unsigned int _component;
  const ExplicitDynamicsContactModel _model;
  const bool _normalize_penalty;

  const Real _tension_release;
  const Real _capture_tolerance;
  bool _update_stateful_data;

  const unsigned int _mesh_dimension;

  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;

  const bool _has_secondary_gap_offset;
  const MooseVariable * const _secondary_gap_offset_var;
  const bool _has_mapped_primary_gap_offset;
  const MooseVariable * const _mapped_primary_gap_offset_var;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * const _aux_solution;

  std::set<dof_id_type> _current_contact_state;
  std::set<dof_id_type> _old_contact_state;

  const bool _print_contact_nodes;
  static Threads::spin_mutex _contact_set_mutex;

  const static unsigned int _no_iterations;

  const MaterialProperty<Real> & _wave_speed;
};
