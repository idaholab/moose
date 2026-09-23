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

class MooseVariableFieldBase;

/**
 * Base class for constraints that tie a dependent degree of freedom to a linear combination of
 * other degrees of freedom by adding rows to the DofMap of the nonlinear system.
 *
 * A multipoint constraint assembles neither a residual nor a Jacobian. libMesh applies the
 * constraint matrix to every element matrix and element vector MOOSE assembles, so a single set of
 * rows reduces the stiffness matrix, the mass matrix and the load vector alike. Derived classes
 * only implement addConstraintRows(), which MultiPointConstraintHub calls every time libMesh
 * rebuilds the constraints of the system.
 */
class MultiPointConstraint : public Constraint, public Coupleable
{
public:
  static InputParameters validParams();

  MultiPointConstraint(const InputParameters & parameters);

  virtual bool usesConstraintRows() const override final { return true; }

  /**
   * Add the rows of this constraint to \p dof_map with
   * libMesh::DofMap::add_constraint_row(). This method is collective: it is called on every rank
   * each time libMesh rebuilds the constraints, and every rank must add the rows of the dependent
   * nodes it has.
   */
  virtual void addConstraintRows(libMesh::DofMap & dof_map) const override = 0;

  virtual void computeResidual() override final {}
  virtual void computeJacobian() override final {}

protected:
  /// A node taking part in a multipoint constraint, with the data needed to build its rows
  struct ConstraintNode
  {
    /// The id of the node
    dof_id_type id;

    /// The coordinates of the node on the undisplaced mesh
    Point point;

    /// The global degree of freedom index of each requested variable at this node, in the order
    /// the variables were requested. An entry is libMesh::DofObject::invalid_id when the variable
    /// has no degree of freedom at the node
    std::vector<dof_id_type> dofs;
  };

  /**
   * @return The ids of the nodes of \p boundary that are present on this rank, which are the nodes
   * whose rows this rank must add
   */
  std::vector<dof_id_type> localNodes(const BoundaryName & boundary) const;

  /**
   * Gather the data of every node of \p boundary on every rank. Each node is contributed by the
   * rank that owns it and the result is sorted by node id, so every rank obtains the same data.
   * @param boundary The node set or side set holding the nodes
   * @param var_numbers The system numbers of the variables whose degrees of freedom are gathered
   */
  std::vector<ConstraintNode>
  gatherBoundaryNodes(const BoundaryName & boundary,
                      const std::vector<unsigned int> & var_numbers) const;

  /**
   * Gather the data of the single node of \p boundary on every rank. It is an error for the
   * boundary to hold any number of nodes other than one.
   * @param boundary The node set or side set holding the node
   * @param var_numbers The system numbers of the variables whose degrees of freedom are gathered
   */
  ConstraintNode gatherSingleNode(const BoundaryName & boundary,
                                  const std::vector<unsigned int> & var_numbers) const;

  /**
   * Gather the single node of \p boundary on every rank and check that every variable of
   * \p variables has a degree of freedom there. It is an error for the boundary to hold any number
   * of nodes other than one, or for one of the variables to have no degree of freedom at that node.
   *
   * Every rank receives the same gathered data, so every rank raises the same error and the run
   * stops instead of hanging. Derived classes that need this check should prefer this overload to
   * repeating it on the dofs of the returned node.
   *
   * @param boundary The node set or side set holding the node
   * @param variables The variables whose degrees of freedom are gathered, in the order the rows of
   * the derived class expect them
   */
  ConstraintNode
  gatherSingleNode(const BoundaryName & boundary,
                   const std::vector<const MooseVariableFieldBase *> & variables) const;
};
