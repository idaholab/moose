//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InclinedNoDisplacementConstraint.h"

#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"
#include "libmesh/fe_base.h"
#include "libmesh/node.h"
#include "libmesh/quadrature_gauss.h"

#include <algorithm>
#include <cmath>
#include <utility>

registerMooseObject("SolidMechanicsApp", InclinedNoDisplacementConstraint);

InputParameters
InclinedNoDisplacementConstraint::validParams()
{
  InputParameters params = MultiPointConstraint::validParams();
  params.addClassDescription(
      "Holds every node of a boundary in the plane of its inclined support, u . n = 0, with one "
      "degree of freedom constraint row per node instead of a penalty term, so that the node "
      "slides freely in the plane and the support adds no stiffness.");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The node sets or side sets of the inclined support");
  params.addRequiredCoupledVar("displacements",
                               "The displacement variables: disp_x and disp_y in two dimensions, "
                               "disp_x, disp_y and disp_z in three");
  params.addParam<RealVectorValue>(
      "normal",
      "The normal of a flat support. When it is omitted, the normal at each node is the area "
      "weighted average of the normals of the boundary faces meeting there, computed once from "
      "the undisplaced mesh.");
  return params;
}

InclinedNoDisplacementConstraint::InclinedNoDisplacementConstraint(
    const InputParameters & parameters)
  : MultiPointConstraint(parameters),
    _boundaries(getParam<std::vector<BoundaryName>>("boundary")),
    _has_constant_normal(isParamValid("normal"))
{
  const auto ndisp = coupledComponents("displacements");

  // A support plane needs at least two displacement components to be a plane, which is the same
  // restriction InclinedNoDisplacementBCAction places on the penalty form
  if (ndisp < 2 || ndisp > 3)
    paramError("displacements",
               "This constraint is specific to 2D and 3D models: two displacement variables are "
               "required in two dimensions and three in three.");

  // getVar() returns null for an entry that is a constant rather than a variable, which is how an
  // input such as displacements = '0 0' arrives here: it counts as two coupled components but
  // couples no variable at all
  for (const auto i : make_range(ndisp))
  {
    _displacements.push_back(getVar("displacements", i));
    if (!_displacements.back())
      paramError("displacements",
                 "Entry ",
                 i,
                 " is not a variable. This constraint ties degrees of freedom, so each entry must "
                 "name a solver variable.");
  }

  if (_has_constant_normal)
  {
    _constant_normal = getParam<RealVectorValue>("normal");
    const auto norm = _constant_normal.norm();
    if (norm == 0)
      paramError("normal", "The normal of the support may not be the zero vector.");

    _constant_normal /= norm;
  }
  else
    computeNodalNormals();
}

void
InclinedNoDisplacementConstraint::computeNodalNormals()
{
  const auto ids = _mesh.getBoundaryIDs(_boundaries);
  const std::set<BoundaryID> support(ids.begin(), ids.end());

  // A constant monomial side rule asks libMesh for nothing but the geometry of the face, which is
  // all a normal needs. It is the rule SideSetsGeneratorBase::setup() builds for the same purpose
  const auto dim = _mesh.getMesh().mesh_dimension();
  const libMesh::FEType fe_type(libMesh::CONSTANT, libMesh::MONOMIAL);
  std::unique_ptr<libMesh::FEBase> fe_face = libMesh::FEBase::build(dim, fe_type);
  libMesh::QGauss qface(dim - 1, libMesh::FIRST);
  fe_face->attach_quadrature_rule(&qface);

  // Quantities have to be requested before the first reinit()
  const std::vector<Point> & normals = fe_face->get_normals();
  const std::vector<Real> & JxW = fe_face->get_JxW();

  // Keyed by support and node, never by node alone: two supports meeting at a node must keep
  // their two normals, or the node would be given one averaged plane instead of both of its own
  using SupportNode = std::pair<BoundaryID, dof_id_type>;
  std::map<SupportNode, RealVectorValue> local_normals;
  std::map<SupportNode, Real> local_areas;
  for (const auto & bnd_elem : *_mesh.getBoundaryElementRange())
  {
    if (!support.count(bnd_elem->_bnd_id))
      continue;

    // Only the rank that owns the element accumulates its face, so the gather below counts every
    // face of the support exactly once however the mesh is partitioned and ghosted
    const Elem * const elem = bnd_elem->_elem;
    if (elem->processor_id() != processor_id())
      continue;

    fe_face->reinit(elem, bnd_elem->_side);

    // Integrating the normal over the face weights it by the area of the face, so a node where
    // faces of different size meet follows the larger face more closely
    RealVectorValue face_normal;
    Real face_area = 0;
    for (const auto qp : index_range(JxW))
    {
      face_normal += JxW[qp] * normals[qp];
      face_area += JxW[qp];
    }

    // A node shared by two faces OF THE SAME support receives the sum of both, which gives it a
    // single averaged normal there and so a single row. That is the roller a user expects: the
    // node stays free to slide along the edge the two faces share. Stacking one row per face
    // instead would lock the node, and would be a separate option rather than a change of this
    // behaviour. Two faces of DIFFERENT supports are a different matter and are kept apart by the
    // key above: each of those planes is a condition of its own
    for (const auto local_node : elem->nodes_on_side(bnd_elem->_side))
    {
      const SupportNode key(bnd_elem->_bnd_id, elem->node_id(local_node));
      local_normals[key] += face_normal;
      local_areas[key] += face_area;
    }
  }

  // The three buffers are gathered in the same rank order, so their entries stay aligned, and
  // every rank then sums the same contributions in the same order. That is what makes the normal,
  // and therefore the row, identical on every rank
  std::vector<BoundaryID> boundary_ids;
  std::vector<dof_id_type> node_ids;
  std::vector<Real> data;
  for (const auto & [key, normal] : local_normals)
  {
    boundary_ids.push_back(key.first);
    node_ids.push_back(key.second);
    for (const auto d : make_range(Moose::dim))
      data.push_back(normal(d));
    data.push_back(libmesh_map_find(local_areas, key));
  }
  _communicator.allgather(boundary_ids);
  _communicator.allgather(node_ids);
  _communicator.allgather(data);

  // One normal and the area it was accumulated over, per gathered support node
  const auto stride = Moose::dim + 1;
  std::map<SupportNode, Real> areas;
  for (const auto i : index_range(node_ids))
  {
    auto & normal = _nodal_normals[boundary_ids[i]][node_ids[i]];
    for (const auto d : make_range(Moose::dim))
      normal(d) += data[i * stride + d];
    areas[SupportNode(boundary_ids[i], node_ids[i])] += data[i * stride + Moose::dim];
  }

  CollectiveError error(*this);

  // Every rank holds the same gathered set, so these conditions are the same everywhere
  for (const auto i : index_range(_boundaries))
    if (!_nodal_normals.count(_mesh.getBoundaryID(_boundaries[i])))
      error.record("The boundary '" + _boundaries[i] +
                   "' holds no faces, so the normal of that support cannot be taken from the "
                   "mesh. Name a side set rather than a node set, or give the 'normal' "
                   "parameter.");

  for (auto & [boundary_id, normals_of_support] : _nodal_normals)
    for (auto & [node_id, normal] : normals_of_support)
    {
      const auto norm = normal.norm();

      // The accumulated normal is at most as long as the accumulated area, so comparing the two
      // is a scale free test: a ratio near zero means the face normals of this support meeting at
      // this node cancel, as they do on a knife edge, and the node has no plane to slide in
      if (norm < libMesh::TOLERANCE * libmesh_map_find(areas, SupportNode(boundary_id, node_id)))
        error.record("The normals of the faces of one support meeting at node " +
                     std::to_string(node_id) +
                     " cancel, so this node has no plane to slide in. Give the 'normal' parameter "
                     "to state the plane of the support.");
      else
        normal /= norm;
    }

  error.raise();
}

void
InclinedNoDisplacementConstraint::addConstraintRows(libMesh::DofMap & dof_map) const
{
  const auto ndisp = _displacements.size();

  std::vector<unsigned int> var_numbers;
  var_numbers.reserve(ndisp);
  for (const auto * const displacement : _displacements)
    var_numbers.push_back(displacement->number());

  // A component that an enabled nodal boundary condition already pins cannot carry a row: libMesh
  // applies a row after the residual form boundary conditions, so the row would override the
  // condition. Such a component may still appear on the right hand side of a row, where the
  // boundary condition simply supplies its value
  std::vector<std::set<dof_id_type>> pinned;
  pinned.reserve(ndisp);
  for (const auto * const displacement : _displacements)
    pinned.push_back(nodesPinnedByNodalBCs(displacement->name()));

  CollectiveError error(*this);

  // Collect, per node, its degrees of freedom and one normal for each support of this object that
  // holds it. A node is visited once, with all of its planes in hand, so that the rows of that
  // node can be eliminated against one another. The supports are read in the order of 'boundary'
  // and std::map hands the nodes back in id order, so every rank walks the same nodes with the
  // same normals in the same order and therefore builds the same rows
  std::map<dof_id_type, std::pair<std::vector<dof_id_type>, std::vector<RealVectorValue>>>
      supported;
  for (const auto & boundary : _boundaries)
  {
    const auto boundary_id = _mesh.getBoundaryID(boundary);

    for (const auto & node : gatherBoundaryNodes(boundary, var_numbers))
    {
      auto & [dofs, node_normals] = supported[node.id];
      dofs = node.dofs;

      if (_has_constant_normal)
      {
        node_normals.push_back(_constant_normal);
        continue;
      }

      const auto & normals_of_support = libmesh_map_find(_nodal_normals, boundary_id);
      const auto it = normals_of_support.find(node.id);
      if (it == normals_of_support.end())
      {
        error.record("The node " + std::to_string(node.id) + " of boundary '" + boundary +
                     "' has no support normal. The normals are taken from the faces of the "
                     "boundaries once, when this constraint is built, so a node the mesh gained "
                     "afterwards has none; give the 'normal' parameter for such a support.");
        continue;
      }

      node_normals.push_back(it->second);
    }
  }

  // One condition of a node: sum(coefficients * u) = rhs
  using Equation = std::pair<libMesh::DofConstraintRow, Number>;

  const auto & constraints = dof_map.get_dof_constraints();
  const auto & rhs_values = dof_map.get_primal_constraint_values();

  // Replace every degree of freedom of \p equation that the DofMap already constrains by the row
  // that defines it. libMesh does not reduce its constraints to independent degrees of freedom
  // until every constraint object has spoken, so a support that shares a node with another support
  // finds that support's row here and eliminates against it. That is what keeps the rows of a node
  // mutually non-recursive, across objects as well as within one: a degree of freedom that is
  // already dependent never survives into the row this builds
  auto substitute = [&constraints, &rhs_values](Equation & equation)
  {
    // Each row written below depends only on degrees of freedom that were independent when it was
    // written, and a dependent one is never referenced again, so the dependencies form a chain
    // rather than a cycle and a handful of passes resolves them. The bound is a guard against a
    // chain this object did not build, not an expected limit
    for (const auto pass : make_range(10u))
    {
      libmesh_ignore(pass);

      libMesh::DofConstraintRow expanded;
      bool found_dependent = false;
      for (const auto & [dof, coefficient] : equation.first)
      {
        const auto it = constraints.find(dof);
        if (it == constraints.end())
        {
          expanded[dof] += coefficient;
          continue;
        }

        found_dependent = true;
        for (const auto & [other_dof, other_coefficient] : it->second)
          expanded[other_dof] += coefficient * other_coefficient;

        // u_dof = sum(other) + value, so moving the substituted term to the left hand side takes
        // its inhomogeneous part off the right hand side
        const auto value = rhs_values.find(dof);
        if (value != rhs_values.end())
          equation.second -= coefficient * value->second;
      }
      equation.first = expanded;

      if (!found_dependent)
        return true;
    }

    return false;
  };

  for (const auto & [node_id, node] : supported)
  {
    const auto & [dofs, node_normals] = node;

    // Every rank that has the node adds its rows, and they all add the same ones
    if (!_mesh.queryNodePtr(node_id))
      continue;

    // gatherBoundaryNodes() reports a variable with no degree of freedom at the node with the
    // invalid id, and reports it on every rank
    bool node_is_usable = true;
    for (const auto i : index_range(_displacements))
      if (dofs[i] == libMesh::DofObject::invalid_id)
      {
        error.record("The displacement variable '" + _displacements[i]->name() +
                     "' has no degree of freedom at node " + std::to_string(node_id) +
                     ". Every node of an inclined support must solve every displacement variable "
                     "of the model.");
        node_is_usable = false;
        break;
      }

    if (!node_is_usable)
      continue;

    for (const auto & normal : node_normals)
    {
      Equation equation;
      for (const auto c : make_range(ndisp))
        equation.first[dofs[c]] = normal(c);
      equation.second = 0;

      if (!substitute(equation))
      {
        error.record("The degrees of freedom of node " + std::to_string(node_id) +
                     " are constrained through a chain too long to resolve here. Another "
                     "constraint of this problem is tying them in a way this object cannot "
                     "eliminate against.");
        break;
      }

      // The substitution leaves the equation scaled arbitrarily, so it is normalized before the
      // pivot is judged, which keeps the test below a relative one
      Real scale = 0;
      for (const auto & [_, coefficient] : equation.first)
        scale = std::max(scale, std::abs(coefficient));

      if (scale == 0)
      {
        // This plane is already implied by the rows of the other supports of the node, which is
        // what two supports naming the same plane come to. An inhomogeneous remainder would mean
        // the planes cannot all hold at once
        if (equation.second != 0.)
          error.record("The supports meeting at node " + std::to_string(node_id) +
                       " cannot all hold at once: their planes, together with the constraints "
                       "already acting on that node, have no common direction of motion.");
        continue;
      }

      // The dependent component is the one the support resists most, among the components of this
      // node that are still available: not already dependent in another row, and not pinned by a
      // nodal boundary condition
      auto pivot = libMesh::invalid_uint;
      Real largest = 0;
      for (const auto c : make_range(ndisp))
      {
        if (pinned[c].count(node_id) || dof_map.is_constrained_dof(dofs[c]))
          continue;

        const auto it = equation.first.find(dofs[c]);
        if (it == equation.first.end())
          continue;

        if (std::abs(it->second) > largest)
        {
          largest = std::abs(it->second);
          pivot = c;
        }
      }

      // Either the boundary conditions and the other rows of this node already decide how it
      // moves through this support, or the support is orthogonal to every component still
      // available. Either way a row here would only fight what already holds
      if (pivot == libMesh::invalid_uint || largest < libMesh::TOLERANCE * scale)
        continue;

      // u_p = - sum_{d != p} (c_d / c_p) u_d, with the right hand side the substitution left
      const auto pivot_dof = dofs[pivot];
      const auto pivot_coefficient = libmesh_map_find(equation.first, pivot_dof);

      libMesh::DofConstraintRow row;
      for (const auto & [dof, coefficient] : equation.first)
        if (dof != pivot_dof)
          row[dof] = -coefficient / pivot_coefficient;

      // Forbid overwriting so that a degree of freedom libMesh already constrains errors out
      // instead of silently losing one of the two constraints. The pivot is chosen among the
      // unconstrained components, so this only ever fires on a bug
      dof_map.add_constraint_row(pivot_dof,
                                 row,
                                 equation.second / pivot_coefficient,
                                 /*forbid_constraint_overwrite=*/true);
    }
  }

  error.raise();
}
