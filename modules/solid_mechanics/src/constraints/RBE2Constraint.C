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
#include "Conversion.h"
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
      "translations and three rotations, as a Nastran RBE2 rigid element does, using "
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
  // This constraint is three-dimensional: a rigid body in three dimensions has three translations
  // and three rotations
  if (coupledComponents("displacements") != 3)
    paramError("displacements",
               "Three displacement variables are required: this constraint is only implemented in "
               "three dimensions.");
  if (coupledComponents("rotations") != 3)
    paramError("rotations",
               "Three rotation variables are required: this constraint is only implemented in "
               "three dimensions.");

  // getVar() returns null for an entry that is a constant rather than a variable, which is how an
  // input such as rotations = '0 0 0' arrives here: it counts as three coupled components but
  // couples no variable at all
  for (const auto i : make_range(3u))
  {
    _displacements.push_back(getVar("displacements", i));
    if (!_displacements.back())
      paramError("displacements",
                 "Entry ",
                 i,
                 " is not a variable. This constraint ties degrees of freedom, so each entry must "
                 "name a solver variable.");

    _rotations.push_back(getVar("rotations", i));
    if (!_rotations.back())
      paramError("rotations",
                 "Entry ",
                 i,
                 " is not a variable. This constraint ties degrees of freedom, so each entry must "
                 "name a solver variable.");
  }

  // The degrees of freedom of the independent node are gathered in this order, so the first three
  // entries of ConstraintNode::dofs are its translations and the last three are its rotations
  _variables = _displacements;
  _variables.insert(_variables.end(), _rotations.begin(), _rotations.end());
}

void
RBE2Constraint::addConstraintRows(libMesh::DofMap & dof_map) const
{
  const auto sys_num = _sys.number();

  // Every rank builds its rows from the independent node, including the ranks that do not have it.
  // This overload also checks that the independent node carries all six degrees of freedom
  const ConstraintNode independent = gatherSingleNode(_independent_boundary, _variables);

  // Whether a dependent node can be used is known only on the ranks that have that node, so the
  // message is agreed on by every rank before it is raised: erroring on a subset of the ranks
  // inside this collective method would leave the other ranks waiting
  auto check_dependent_node = [&](const Node & node, std::string & param_name) -> std::string
  {
    if (node.id() == independent.id)
    {
      param_name = "dependent_boundary";
      return "The node " + Moose::stringify(node.id()) + " of boundary '" + _dependent_boundary +
             "' is the independent node of boundary '" + _independent_boundary +
             "'. A node cannot be tied rigidly to itself.";
    }

    for (const auto i : make_range(3u))
      if (node.n_comp(sys_num, _displacements[i]->number()) == 0)
      {
        param_name = "displacements";
        return "The displacement variable '" + _displacements[i]->name() +
               "' has no degree of freedom at the dependent node " + Moose::stringify(node.id()) +
               " of boundary '" + _dependent_boundary +
               "'. Every dependent node of an RBE2 constraint must solve the three displacement "
               "variables.";
      }

    if (_tie_rotations)
      for (const auto i : make_range(3u))
        if (node.n_comp(sys_num, _rotations[i]->number()) == 0)
        {
          param_name = "dependent_dofs";
          return "The rotation variable '" + _rotations[i]->name() +
                 "' has no degree of freedom at the dependent node " + Moose::stringify(node.id()) +
                 " of boundary '" + _dependent_boundary +
                 "'. Use 'dependent_dofs = translations' when the dependent nodes carry "
                 "translations only, as continuum nodes do.";
        }

    return std::string("");
  };

  std::string local_error;
  std::string local_error_param;

  for (const auto node_id : localNodes(_dependent_boundary))
  {
    const Node & node = _mesh.nodeRef(node_id);

    local_error = check_dependent_node(node, local_error_param);
    if (!local_error.empty())
      break;

    // The lever arm of the dependent node, taken from the undisplaced mesh. It is zero when the
    // dependent node is a distinct node at the position of the independent node, a splice, and the
    // rows then reduce to u_k = u_0
    const Point r = node - independent.point;

    for (const auto i : make_range(3u))
    {
      // u_k = u_0 + theta_0 x r_k, whose x component is u_{0,x} + theta_{0,y} r_{k,z} -
      // theta_{0,z} r_{k,y}, and cyclic for the y and z components
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

  std::vector<std::string> errors;
  std::vector<std::string> error_params;
  _communicator.allgather(local_error, errors);
  _communicator.allgather(local_error_param, error_params);
  for (const auto i : index_range(errors))
    if (!errors[i].empty())
      paramError(error_params[i], errors[i]);
}
