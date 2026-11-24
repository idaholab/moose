//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Constraint.h"
#include "Coupleable.h"
#include "MooseVariableFE.h"

/**
 * Base class for constraints that tie a dependent degree of freedom to a linear combination of
 * other degrees of freedom with exact constraints in the DofMap. It assembles no residual and no
 * Jacobian.
 */
class MultiPointConstraint : public Constraint, public Coupleable
{
public:
  static InputParameters validParams();

  MultiPointConstraint(const InputParameters & parameters);

  /// The private parameter naming the variable parameter that determines the nonlinear system
  static constexpr auto system_variable_param = "_system_variable_param";

  virtual bool usesExactConstraints() const override final { return true; }

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override = 0;

  virtual void computeResidual() override final {}
  virtual void computeJacobian() override final {}

protected:
  /// A node taking part in a multipoint constraint
  struct ConstraintNode
  {
    /// The id of the node
    dof_id_type id;

    /// The coordinates of the node on the undisplaced mesh
    Point point;

    /// The dof of each requested variable at this node, or invalid_id where it has none
    std::vector<dof_id_type> dofs;
  };

  /**
   * Get the variables of a coupled variable parameter whose degrees of freedom this constraint
   * ties. Raises an error for an entry that is a constant, and for a variable that is not in the
   * nonlinear system of this constraint. Coupleable::getVar() raises the error for a vector, array
   * or finite volume variable.
   * @param param_name The name of the coupled variable parameter
   * @return The variables, in the order of the input
   */
  std::vector<const MooseVariable *> getConstrainedVars(const std::string & param_name);

  /**
   * @param boundary The node set or side set holding the nodes
   * @return The ids of the nodes of the boundary that this rank owns
   */
  std::vector<dof_id_type> ownedNodes(const BoundaryName & boundary) const;

  /**
   * Gather every node of a boundary on every rank, sorted by id. Every rank must call this.
   * @param boundary The node set or side set holding the nodes
   * @param variables The variables to gather the dofs of
   * @return The gathered nodes
   */
  std::vector<ConstraintNode>
  gatherBoundaryNodes(const BoundaryName & boundary,
                      const std::vector<const MooseVariable *> & variables) const;

  /**
   * Gather the single node of a boundary on every rank and check that every variable has a dof
   * there. Every rank must call this.
   * @param boundary The node set or side set holding the node
   * @param variables The variables to gather the dofs of
   * @return The gathered node
   */
  ConstraintNode gatherSingleNode(const BoundaryName & boundary,
                                  const std::vector<const MooseVariable *> & variables) const;
};
