//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RBE3Constraint.h"

#include "MooseVariableFieldBase.h"
#include "RankTwoTensor.h"

#include "libmesh/dof_map.h"
#include "libmesh/dof_object.h"

#include <algorithm>
#include <cmath>
#include <set>

registerMooseObject("SolidMechanicsApp", RBE3Constraint);

namespace
{
/// The skew-symmetric matrix [a]_x, which maps a vector b to the cross product a x b
RankTwoTensor
skew(const Point & a)
{
  RankTwoTensor s;
  s(0, 1) = -a(2);
  s(0, 2) = a(1);
  s(1, 0) = a(2);
  s(1, 2) = -a(0);
  s(2, 0) = -a(1);
  s(2, 1) = a(0);
  return s;
}
}

InputParameters
RBE3Constraint::validParams()
{
  InputParameters params = MultiPointConstraint::validParams();
  params.addClassDescription(
      "Constrains a reference node to the weighted least-squares rigid-body fit of the "
      "displacements of a set of independent nodes, which distributes a load applied at the "
      "reference node over those nodes without adding stiffness, as a Nastran RBE3 element does.");

  params.addRequiredParam<BoundaryName>("reference_boundary",
                                        "The node set holding the single reference node, whose "
                                        "degrees of freedom this constraint makes dependent");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "independent_boundaries",
      "The node sets whose nodes form the independent set of this constraint. Only the "
      "displacements of these nodes are used.");
  params.addParam<std::vector<Real>>("weights",
                                     "The weight of each entry of 'independent_boundaries', "
                                     "applied to every node of that node set. Every weight is one "
                                     "when this parameter is not given.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables. This constraint supports three-dimensional models only.");
  params.addCoupledVar("rotations",
                       "The three rotation variables. When they are given, the rotations of the "
                       "reference node are constrained to the rotation of the least-squares fit of "
                       "the independent displacements.");

  return params;
}

RBE3Constraint::RBE3Constraint(const InputParameters & parameters)
  : MultiPointConstraint(parameters),
    _reference_boundary(getParam<BoundaryName>("reference_boundary")),
    _independent_boundaries(getParam<std::vector<BoundaryName>>("independent_boundaries")),
    _weights(isParamValid("weights") ? getParam<std::vector<Real>>("weights")
                                     : std::vector<Real>(_independent_boundaries.size(), 1.0))
{
  if (_independent_boundaries.empty())
    paramError("independent_boundaries", "At least one node set must be given.");

  if (_weights.size() != _independent_boundaries.size())
    paramError("weights",
               "This constraint was given ",
               _weights.size(),
               " weights but ",
               _independent_boundaries.size(),
               " entries in 'independent_boundaries'. One weight per node set is required.");

  for (const auto weight : _weights)
    if (weight <= 0.0)
      paramError("weights",
                 "Every weight must be positive: a weight is the relative share with which an "
                 "independent node takes part in the averaged motion.");

  if (coupledComponents("displacements") != 3)
    paramError("displacements",
               "Exactly three displacement variables are required: this constraint supports "
               "three-dimensional models only.");
  _displacement_vars = getFieldVars("displacements");

  if (isCoupled("rotations"))
  {
    if (coupledComponents("rotations") != 3)
      paramError("rotations",
                 "Exactly three rotation variables are required: this constraint supports "
                 "three-dimensional models only.");
    _rotation_vars = getFieldVars("rotations");
  }

  // Only the displacements are gathered on the independent nodes, while the reference node is
  // gathered with its displacements followed by its rotations, which is the order of its rows
  for (const auto * const var : _displacement_vars)
    _displacement_var_numbers.push_back(var->number());
  _reference_vars = _displacement_vars;
  _reference_vars.insert(_reference_vars.end(), _rotation_vars.begin(), _rotation_vars.end());
}

const MooseVariableBase &
RBE3Constraint::variable() const
{
  return *_displacement_vars[0];
}

void
RBE3Constraint::addConstraintRows(libMesh::DofMap & dof_map) const
{
  // Every gather below is collective and returns the same data on every rank, so all ranks build
  // the same coefficients and reach the same errors. Only the ranks that have the reference node
  // add the rows.
  const auto reference = gatherSingleNode(_reference_boundary, _reference_vars);

  // Collect the independent nodes of every node set, with the weight of the node set they come
  // from, and reject a node that is listed more than once
  std::vector<ConstraintNode> independent_nodes;
  std::vector<Real> weights;
  std::set<dof_id_type> collected_ids;
  for (const auto b : index_range(_independent_boundaries))
    for (const auto & node :
         gatherBoundaryNodes(_independent_boundaries[b], _displacement_var_numbers))
    {
      if (node.id == reference.id)
        mooseError("Node ",
                   node.id,
                   " of boundary '",
                   _independent_boundaries[b],
                   "' is the reference node of this constraint. The reference node cannot be one "
                   "of the independent nodes it is constrained to.");

      if (!collected_ids.insert(node.id).second)
        mooseError("Node ",
                   node.id,
                   " belongs to more than one node set of 'independent_boundaries'. A node can "
                   "carry only one weight, so it can appear in the independent set only once.");

      for (const auto c : index_range(_displacement_vars))
        if (node.dofs[c] == libMesh::DofObject::invalid_id)
          mooseError("The displacement variable '",
                     _displacement_vars[c]->name(),
                     "' has no degree of freedom at node ",
                     node.id,
                     " of boundary '",
                     _independent_boundaries[b],
                     "', which is one of the independent nodes of this constraint.");

      independent_nodes.push_back(node);
      weights.push_back(_weights[b]);
    }

  if (independent_nodes.empty())
    mooseError("The node sets given in 'independent_boundaries' hold no nodes, so this constraint "
               "has no independent set.");

  // The weighted centroid of the independent nodes and the position of the reference node
  // relative to it
  Real total_weight = 0.0;
  Point centroid;
  for (const auto i : index_range(independent_nodes))
  {
    total_weight += weights[i];
    centroid += independent_nodes[i].point * weights[i];
  }
  centroid /= total_weight;
  const Point d = reference.point - centroid;

  // The weighted second-moment tensor J = sum_i w_i (|r_i|^2 I - r_i r_i^T)
  RankTwoTensor moment;
  for (const auto i : index_range(independent_nodes))
  {
    const Point r = independent_nodes[i].point - centroid;
    RankTwoTensor contribution = RankTwoTensor::Identity() * r.norm_sq();
    contribution -= RankTwoTensor::selfOuterProduct(r);
    moment += contribution * weights[i];
  }

  // The rotation of the least-squares fit only enters the rows when the reference node is away
  // from the weighted centroid or when its rotations are constrained. The radius of gyration of
  // the independent set, sqrt(trace(J) / (2 W)), is the length this compares the offset with.
  const Real radius = std::sqrt(moment.trace() / (2.0 * total_weight));
  const bool uses_rotation = !_rotation_vars.empty() || d.norm() > libMesh::TOLERANCE * radius;

  RankTwoTensor moment_inverse;
  if (uses_rotation)
  {
    // The determinant of J scales with the sixth power of a length and its trace with the second,
    // so this ratio is dimensionless. It vanishes when the independent nodes are collinear or when
    // there is only one of them, and the rotation of the fit is then undefined. The threshold is
    // loose enough to catch a set that is collinear to roundoff and far below the value of any set
    // that is not.
    const Real scale = moment.trace() / 3.0;
    if (moment.det() <= 1.0e-10 * scale * scale * scale)
      mooseError("The weighted second-moment tensor of the independent nodes of this constraint is "
                 "singular, so the rotation of their least-squares fit is not defined. The "
                 "independent nodes are collinear, or there is only one of them. Give three or "
                 "more independent nodes that are not on a line, or place the reference node at "
                 "their weighted centroid and leave 'rotations' unset.");

    moment_inverse = moment.inverse();
  }

  // Everything above is collective and everything below is local, so the ranks that do not have
  // the reference node are done here instead of building rows they would discard
  const auto local_reference_nodes = localNodes(_reference_boundary);
  if (std::find(local_reference_nodes.begin(), local_reference_nodes.end(), reference.id) ==
      local_reference_nodes.end())
    return;

  const RankTwoTensor skew_d = skew(d);

  // One row per constrained degree of freedom of the reference node: the three displacements,
  // followed by the three rotations when they are given
  std::vector<libMesh::DofConstraintRow> rows(_reference_vars.size());
  for (const auto i : index_range(independent_nodes))
  {
    const auto & node = independent_nodes[i];
    const Point r = node.point - centroid;

    // d(theta_ref)/d(u_i) = w_i J^{-1} [r_i]_x
    RankTwoTensor rotation_block;
    if (uses_rotation)
      rotation_block = (moment_inverse * skew(r)) * weights[i];

    // d(u_ref)/d(u_i) = (w_i / W) I - w_i [d]_x J^{-1} [r_i]_x
    RankTwoTensor displacement_block = RankTwoTensor::Identity() * (weights[i] / total_weight);
    displacement_block -= skew_d * rotation_block;

    for (const auto a : make_range(3u))
      for (const auto b : make_range(3u))
      {
        if (displacement_block(a, b) != 0.0)
          rows[a][node.dofs[b]] += displacement_block(a, b);

        if (!_rotation_vars.empty() && rotation_block(a, b) != 0.0)
          rows[a + _displacement_vars.size()][node.dofs[b]] += rotation_block(a, b);
      }
  }

  for (const auto v : index_range(rows))
    dof_map.add_constraint_row(reference.dofs[v], rows[v], /*forbid_constraint_overwrite=*/true);
}
