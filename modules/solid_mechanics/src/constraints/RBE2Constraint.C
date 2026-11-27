//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RBE2Constraint.h"

#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/node.h"

registerMooseObject("SolidMechanicsApp", RBE2Constraint);

InputParameters
RBE2Constraint::validParams()
{
  InputParameters params = MultiPointConstraint::validParams();
  params.addClassDescription(
      "Ties a set of dependent nodes rigidly to a single independent node carrying three "
      "translations and three rotations, using "
      "small-rotation kinematics on the undisplaced mesh.");
  params.addRequiredParam<BoundaryName>(
      "independent_boundary",
      "The node set holding the single independent node whose six degrees of freedom drive the "
      "rigid body");
  params.addRequiredParam<BoundaryName>(
      "dependent_boundary",
      "The node set or side set whose nodes move rigidly with the independent node");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables of the dependent nodes and of the independent node");
  params.addRequiredCoupledVar("rotations",
                               "The three rotation variables that drive the rigid body. The "
                               "independent node must carry a degree of freedom of each of them.");
  MooseEnum dependent_dofs("translations translations_and_rotations", "translations");
  params.addParam<MooseEnum>("dependent_dofs",
                             dependent_dofs,
                             "The degrees of freedom of the dependent nodes that are tied to the "
                             "independent node; use 'translations_and_rotations' when the "
                             "dependent nodes carry rotations, as beam nodes do");
  return params;
}

RBE2Constraint::RBE2Constraint(const InputParameters & parameters)
  : MultiPointConstraint(parameters),
    _independent_boundary(getParam<BoundaryName>("independent_boundary")),
    _dependent_boundary(getParam<BoundaryName>("dependent_boundary")),
    _tie_rotations(getParam<MooseEnum>("dependent_dofs") == "translations_and_rotations")
{
  if (coupledComponents("displacements") != 3)
    paramError("displacements",
               "Three displacement variables are required: this constraint is only implemented in "
               "three dimensions.");
  if (coupledComponents("rotations") != 3)
    paramError("rotations",
               "Three rotation variables are required: this constraint is only implemented in "
               "three dimensions.");

  _displacements = getConstrainedVars("displacements");
  _rotations = getConstrainedVars("rotations");

  _variables = _displacements;
  _variables.insert(_variables.end(), _rotations.begin(), _rotations.end());
}

void
RBE2Constraint::addExactConstraints(libMesh::DofMap & dof_map) const
{
  const auto sys_num = _sys.number();

  const ConstraintNode independent = gatherSingleNode(_independent_boundary, _variables);

  for (const auto node_id : ownedNodes(_dependent_boundary))
  {
    const Node & node = _mesh.nodeRef(node_id);

    if (node.id() == independent.id)
      paramError("dependent_boundary",
                 "The node ",
                 node.id(),
                 " of boundary '",
                 _dependent_boundary,
                 "' is the independent node of boundary '",
                 _independent_boundary,
                 "'. A node cannot be tied rigidly to itself.");

    for (const auto * const displacement : _displacements)
      if (node.n_comp(sys_num, displacement->number()) == 0)
        paramError("displacements",
                   "The displacement variable '",
                   displacement->name(),
                   "' has no degree of freedom at the dependent node ",
                   node.id(),
                   " of boundary '",
                   _dependent_boundary,
                   "'. Every dependent node of an RBE2 constraint must solve the three "
                   "displacement variables.");

    if (_tie_rotations)
      for (const auto * const rotation : _rotations)
        if (node.n_comp(sys_num, rotation->number()) == 0)
          paramError("dependent_dofs",
                     "The rotation variable '",
                     rotation->name(),
                     "' has no degree of freedom at the dependent node ",
                     node.id(),
                     " of boundary '",
                     _dependent_boundary,
                     "'. Use 'dependent_dofs = translations' when the dependent nodes carry "
                     "translations only, as continuum nodes do.");

    // The lever arm on the undisplaced mesh; zero for a splice, where u_k = u_0
    const Point r = node - independent.point;

    for (const auto i : make_range(3u))
    {
      // u_k = u_0 + theta_0 x r_k
      const auto j = (i + 1) % 3;
      const auto k = (i + 2) % 3;

      libMesh::DofConstraintRow row;
      row[independent.dofs[i]] = 1.0;
      row[independent.dofs[3 + j]] = r(k);
      row[independent.dofs[3 + k]] = -r(j);

      dof_map.add_constraint_row(node.dof_number(sys_num, _displacements[i]->number(), 0),
                                 row,
                                 /*forbid_constraint_overwrite=*/true);
    }

    if (_tie_rotations)
      for (const auto i : make_range(3u))
      {
        // theta_k = theta_0 under small rotations
        libMesh::DofConstraintRow row;
        row[independent.dofs[3 + i]] = 1.0;

        dof_map.add_constraint_row(node.dof_number(sys_num, _rotations[i]->number(), 0),
                                   row,
                                   /*forbid_constraint_overwrite=*/true);
      }
  }
}
