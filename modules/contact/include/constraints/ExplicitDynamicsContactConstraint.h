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
#include "Coupleable.h"

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
  virtual void residualEnd() override {}

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

  bool shouldApply() override;
  void computeContactForce(const Node & node, PenetrationInfo * pinfo, bool update_contact_set);
  virtual bool isExplicitConstraint() const override { return true; }
  /**
   * Return false so that the nonlinear system does not try to add Jacobian entries
   * from the contact forces.
   * @return bool indicating whether we need to couple Jacobian entries
   */
  virtual bool addCouplingEntriesToJacobian() override { return false; }

  virtual const std::unordered_set<unsigned int> & getMatPropDependencies() const override;

  virtual void overwriteBoundaryVariables(NumericVector<Number> & soln,
                                          const Node & secondary_node) const override;

protected:
  /**
   * Determine "Lagrange multipliers" from the iterative solution of the impact problem.
   * @param node The number of the variable to be checked
   * @param pinfo The component index computed in this routine
   * @param distance_gap The gap distance at the constraint node
   */
  void solveImpactEquations(const Node & node,
                            PenetrationInfo * pinfo,
                            const RealVectorValue & distance_gap);

  MooseSharedPointer<DisplacedProblem> _displaced_problem;
  Real gapOffset(const Node & node);
  Real nodalArea(const Node & node);
  Real getPenalty(const Node & node);

  const unsigned int _component;
  const ExplicitDynamicsContactModel _model;

  bool _update_stateful_data;

  const unsigned int _mesh_dimension;

  std::vector<unsigned int> _vars;
  std::vector<MooseVariable *> _var_objects;

  const bool _has_secondary_gap_offset;
  const MooseVariable * const _secondary_gap_offset_var;
  const bool _has_mapped_primary_gap_offset;
  const MooseVariable * const _mapped_primary_gap_offset_var;

  MooseVariable * _nodal_area_var;
  const MooseVariable * _nodal_density_var;
  const MooseVariable * _nodal_wave_speed_var;

  SystemBase & _aux_system;
  const NumericVector<Number> * const _aux_solution;
  const Real _penalty;

  const bool _print_contact_nodes;

  const static unsigned int _no_iterations;

  NumericVector<Number> & _residual_copy;

  /// Density material for neighbor projection
  const MaterialProperty<Real> & _neighbor_density;

  /// Wave speed material for neighbor projection
  const MaterialProperty<Real> & _neighbor_wave_speed;

  /// Nodal gap rate (output for debugging or analyst perusal)
  MooseWritableVariable * _gap_rate;

  /// X component of velocity at the closest point
  const VariableValue & _neighbor_vel_x;
  /// Y component of velocity at the closest point
  const VariableValue & _neighbor_vel_y;
  /// Z component of velocity at the closest point
  const VariableValue & _neighbor_vel_z;

  /// Whether to overwrite contact boundary nodal solution
  const bool _overwrite_current_solution;

private:
  std::unordered_map<dof_id_type, Real> _dof_to_position;
};

inline const std::unordered_set<unsigned int> &
ExplicitDynamicsContactConstraint::getMatPropDependencies() const
{
  return TwoMaterialPropertyInterface::getMatPropDependencies();
}
