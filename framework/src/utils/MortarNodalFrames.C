//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarNodalFrames.h"
#include "AutomaticMortarGeneration.h"
#include "MooseLagrangeHelpers.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

namespace Moose
{
namespace Mortar
{
ADPoint
nodalCoordinate(const Node & node, const std::array<const MooseVariable *, 3> & disp_vars)
{
  ADPoint point = node;

  for (const auto component : index_range(disp_vars))
    if (const auto * const variable = disp_vars[component])
    {
      const auto sys_num = variable->sys().number();
      const auto var_num = variable->number();
      // Enriched elements can have mapping nodes without independent displacement degrees of
      // freedom. Their positions enter the geometric map, but they contribute no independent
      // displacement derivative.
      if (node.n_dofs(sys_num, var_num))
        Moose::derivInsert(
            point(component).derivatives(), node.dof_number(sys_num, var_num, 0), 1.0);
    }

  return point;
}

bool
supportsNodalNormalDerivatives(const std::array<const MooseVariable *, 3> & disp_vars,
                               const SystemBase & sys)
{
  for (const auto * const variable : disp_vars)
    if (variable && (!variable->isNodal() || &variable->sys() != &sys))
      return false;
  return true;
}

void
checkNodalNormalDisplacements(const MooseBase & object,
                              const std::array<const MooseVariable *, 3> & disp_vars,
                              const SystemBase & sys)
{
  const std::array<const char *, 3> parameter_names{{"disp_x", "disp_y", "disp_z"}};
  for (const auto component : index_range(disp_vars))
    if (const auto * const variable = disp_vars[component])
    {
      if (!variable->isNodal())
        object.paramError(parameter_names[component],
                          "Nodal-normal derivatives require a nodal displacement variable.");
      if (&variable->sys() != &sys)
        object.paramError(parameter_names[component],
                          "Nodal-normal derivatives require displacement variables in the "
                          "nonlinear system assembled by this contact object.");
    }
}

const ADRealVectorValue &
ADNodalFrames::normal(const AutomaticMortarGeneration & amg,
                      const Node & node,
                      const Point & stored_normal,
                      const bool do_derivatives)
{
  if (const auto it = _normals.find(&node); it != _normals.end())
    return it->second;

  if (!do_derivatives)
    return _normals.emplace(&node, stored_normal).first->second;

  amg.computeADNodalNormals([this](const Node & coordinate_node)
                            { return nodalCoordinate(coordinate_node, _disp_vars); },
                            _normals);
  return libmesh_map_find(_normals, &node);
}

const std::array<ADRealVectorValue, 2> &
ADNodalFrames::tangents(const AutomaticMortarGeneration & amg,
                        const Node & node,
                        const Point & stored_normal,
                        const bool do_derivatives)
{
  auto it = _tangents.find(&node);
  if (it == _tangents.end())
    it = _tangents
             .emplace(&node,
                      Moose::Mortar::householderTangents(
                          normal(amg, node, stored_normal, do_derivatives)))
             .first;
  return it->second;
}

std::vector<ADRealVectorValue>
ADNodalFrames::interpolatedNormals(const AutomaticMortarGeneration & amg,
                                   const Elem & secondary_elem,
                                   const std::vector<Point> & xi_pts,
                                   const std::vector<Point> & stored_normals,
                                   const bool do_derivatives)
{
  mooseAssert(xi_pts.size() == stored_normals.size(),
              "There must be one stored normal per reference point.");
  if (!do_derivatives)
    return {stored_normals.begin(), stored_normals.end()};

  const auto stored_nodal_normals = amg.getNodalNormals(secondary_elem);
  const auto mortar_dim = secondary_elem.dim();
  std::vector<ADRealVectorValue> normals(xi_pts.size());
  for (const auto n : make_range(secondary_elem.n_nodes()))
  {
    const auto & nodal_normal =
        normal(amg, secondary_elem.node_ref(n), stored_nodal_normals[n], do_derivatives);
    for (const auto qp : index_range(xi_pts))
    {
      const auto phi =
          (mortar_dim == 1)
              ? Moose::fe_lagrange_1D_shape(secondary_elem.default_order(), n, xi_pts[qp](0))
              : Moose::fe_lagrange_2D_shape(secondary_elem.type(),
                                            secondary_elem.default_order(),
                                            n,
                                            cast_ref<const TypeVector<Real> &>(xi_pts[qp]));
      normals[qp] += phi * nodal_normal;
    }
  }

  // getNormals() flips the interpolated normals of a periodic interface. Matching the stored
  // values keeps the residual unchanged whichever convention the interface uses.
  for (const auto qp : index_range(normals))
    if (MetaPhysicL::raw_value(normals[qp]) * stored_normals[qp] < 0)
      normals[qp] *= -1;

  return normals;
}

void
ADNodalFrames::clear()
{
  _normals.clear();
  _tangents.clear();
}
}
}
