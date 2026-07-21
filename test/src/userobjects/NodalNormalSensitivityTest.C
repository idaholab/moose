//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalSensitivityTest.h"

#include "AutomaticMortarGeneration.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MortarInterfaceWarehouse.h"

#include "libmesh/enum_to_string.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>

registerMooseObject("MooseTestApp", NodalNormalSensitivityTest);

InputParameters
NodalNormalSensitivityTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "collapse_secondary_element",
      false,
      "Whether to collapse one secondary element and verify that sensitivity construction errors.");
  params.addParam<bool>(
      "representative_face_only",
      false,
      "Whether to check the nodes of one representative secondary face instead of the complete "
      "interface. Incident faces for those nodes are still included in each sensitivity stencil.");
  params.addRangeCheckedParam<Real>(
      "coordinate_scale",
      1.0,
      "coordinate_scale > 0",
      "Uniform scale applied to the test mesh before checking the sensitivities.");
  params.addRangeCheckedParam<Real>(
      "relative_step",
      1e-7,
      "relative_step > 0",
      "Coordinate perturbation relative to the local secondary-element size.");
  params.addParam<Real>("relative_tolerance", 2e-5, "Relative FD comparison tolerance.");
  params.addParam<Real>(
      "absolute_tolerance",
      2e-8,
      "Absolute FD comparison tolerance after scaling by the inverse local element size.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.addClassDescription(
      "Checks averaged mortar nodal-normal coordinate sensitivities by central differences.");
  return params;
}

NodalNormalSensitivityTest::NodalNormalSensitivityTest(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _collapse_secondary_element(getParam<bool>("collapse_secondary_element")),
    _representative_face_only(getParam<bool>("representative_face_only")),
    _coordinate_scale(getParam<Real>("coordinate_scale")),
    _relative_step(getParam<Real>("relative_step")),
    _relative_tolerance(getParam<Real>("relative_tolerance")),
    _absolute_tolerance(getParam<Real>("absolute_tolerance"))
{
}

void
NodalNormalSensitivityTest::execute()
{
  const auto & interfaces = _fe_problem.getMortarInterfaces(/*displaced=*/false);
  if (interfaces.size() != 1)
    mooseError("NodalNormalSensitivityTest expects exactly one mortar interface, but found ",
               interfaces.size(),
               ".");

  auto & amg = *interfaces.begin()->second.amg;
  auto & mesh = _fe_problem.mesh().getMesh();

  if (_collapse_secondary_element)
  {
    const auto & node_to_secondary_elem = amg.nodesToSecondaryElem();
    if (node_to_secondary_elem.empty() || node_to_secondary_elem.begin()->second.empty())
      mooseError("Unable to find a secondary mortar element to collapse.");

    Elem * const secondary_elem =
        const_cast<Elem *>(node_to_secondary_elem.begin()->second.front());
    const Point collapsed_point = secondary_elem->point(0);
    for (const auto node_index : make_range(secondary_elem->n_nodes()))
      secondary_elem->point(node_index) = collapsed_point;

    amg.getNodalNormalCoordinateSensitivity(*secondary_elem->node_ptr(0));
    mooseError("Collapsing a secondary mortar element did not trigger a degeneracy error.");
  }

  std::vector<std::pair<Node *, Point>> original_node_locations;
  if (!MooseUtils::absoluteFuzzyEqual(_coordinate_scale, 1.0))
  {
    original_node_locations.reserve(mesh.n_nodes());
    for (auto * const node : mesh.node_ptr_range())
    {
      original_node_locations.emplace_back(node, *node);
      for (const auto component : make_range(LIBMESH_DIM))
        (*node)(component) *= _coordinate_scale;
    }
    amg.computeNodalGeometry();
  }

  // This map is built from the secondary interface connectivity, independently of the analytic
  // sensitivity stencil. Every node on every incident face is therefore perturbed, including
  // candidates that the implementation may have accidentally omitted from its stencil.
  std::map<dof_id_type, std::vector<const Elem *>> normal_node_to_incident_elems;
  for (const auto & [node_id, incident_elems] : amg.nodesToSecondaryElem())
    if (!incident_elems.empty())
      normal_node_to_incident_elems.emplace(node_id, incident_elems);

  if (normal_node_to_incident_elems.empty())
    mooseError("Unable to find secondary mortar nodes to test.");

  if (_representative_face_only)
  {
    const Elem & representative_face = *normal_node_to_incident_elems.begin()->second.front();
    std::set<dof_id_type> representative_node_ids;
    for (const auto & node : representative_face.node_ref_range())
      representative_node_ids.insert(node.id());

    for (auto it = normal_node_to_incident_elems.begin();
         it != normal_node_to_incident_elems.end();)
      if (!representative_node_ids.count(it->first))
        it = normal_node_to_incident_elems.erase(it);
      else
        ++it;
  }

  // Copy every analytic stencil before the coordinate perturbations below rebuild nodal geometry
  // and invalidate the cached stencils. Node pointers remain stable across those rebuilds.
  std::map<dof_id_type, AutomaticMortarGeneration::NodalNormalSensitivityStencil>
      normal_node_to_analytic_stencil;
  for (const auto & [normal_node_id, incident_elems] : normal_node_to_incident_elems)
  {
    libmesh_ignore(incident_elems);
    normal_node_to_analytic_stencil.emplace(
        normal_node_id, amg.getNodalNormalCoordinateSensitivity(mesh.node_ref(normal_node_id)));
  }

  std::set<ElemType> tested_elem_types;
  std::size_t tested_nodes = 0;
  std::size_t tested_coordinate_perturbations = 0;
  for (const auto & [normal_node_id, incident_elems] : normal_node_to_incident_elems)
  {
    const Node * const normal_node = mesh.node_ptr(normal_node_id);
    if (!normal_node)
      mooseError("Unable to find secondary mortar node ", normal_node_id, " in the mesh.");

    const Elem * const representative_elem = incident_elems.front();
    const auto normal_node_index = representative_elem->get_node_index(normal_node);
    if (normal_node_index == libMesh::invalid_uint)
      mooseError("Secondary mortar element ",
                 representative_elem->id(),
                 " does not contain its mapped normal node ",
                 normal_node_id,
                 ".");

    const Point baseline_normal = amg.getNodalNormals(*representative_elem)[normal_node_index];
    const auto & stencil = libmesh_map_find(normal_node_to_analytic_stencil, normal_node_id);
    const unsigned int dimension = amg.dim();

    if (std::abs(baseline_normal.norm() - 1.0) > 100 * std::numeric_limits<Real>::epsilon())
      mooseError(
          "The baseline mortar nodal normal at node ", normal_node_id, " is not a unit vector.");

    std::map<dof_id_type, const Node *> candidate_nodes;
    Real local_size = 0;
    for (const auto * const incident_elem : incident_elems)
    {
      if (!incident_elem)
        mooseError("The secondary mortar node-to-element map contains a null element.");

      if (incident_elem->get_node_index(normal_node) == libMesh::invalid_uint)
        mooseError("Secondary mortar element ",
                   incident_elem->id(),
                   " in the incident-face map does not contain node ",
                   normal_node_id,
                   ".");

      local_size = std::max(local_size, incident_elem->hmax());
      tested_elem_types.insert(incident_elem->type());
      for (const auto side_node_index : make_range(incident_elem->n_nodes()))
        candidate_nodes.emplace(incident_elem->node_id(side_node_index),
                                incident_elem->node_ptr(side_node_index));
    }

    if (local_size <= 0)
      mooseError("The secondary face star at node ",
                 normal_node_id,
                 " has a non-positive characteristic length.");

    std::map<dof_id_type, TensorValue<Real>> analytic_stencil;
    for (const auto & sensitivity : stencil)
    {
      if (!sensitivity.node)
        mooseError("The nodal-normal sensitivity stencil at secondary node ",
                   normal_node_id,
                   " contains a null coordinate node.");
      if (!candidate_nodes.count(sensitivity.node->id()))
        mooseError("The nodal-normal sensitivity stencil at secondary node ",
                   normal_node_id,
                   " contains spurious node ",
                   sensitivity.node->id(),
                   ", which is not on an incident secondary face.");
      if (!analytic_stencil.emplace(sensitivity.node->id(), sensitivity.dnormal_dnode_coordinate)
               .second)
        mooseError("The nodal-normal sensitivity stencil at secondary node ",
                   normal_node_id,
                   " contains duplicate entries for coordinate node ",
                   sensitivity.node->id(),
                   ".");
    }

    for (const auto & [candidate_node_id, candidate_node] : candidate_nodes)
      for (const auto coordinate : make_range(dimension))
      {
        Point analytic_derivative;
        const auto analytic_it = analytic_stencil.find(candidate_node_id);
        if (analytic_it != analytic_stencil.end())
          for (const auto component : make_range(LIBMESH_DIM))
            analytic_derivative(component) = analytic_it->second(component, coordinate);

        const Real tangent_tolerance = 1000 * std::numeric_limits<Real>::epsilon() *
                                       std::max(1.0 / local_size, analytic_derivative.norm());
        if (std::abs(baseline_normal * analytic_derivative) > tangent_tolerance)
          mooseError("The nodal-normal derivative at secondary node ",
                     normal_node_id,
                     " with respect to coordinate ",
                     coordinate,
                     " of node ",
                     candidate_node_id,
                     " is not tangent to the unit sphere.");

        Node & coordinate_node = const_cast<Node &>(*candidate_node);
        const Real original_coordinate = coordinate_node(coordinate);
        const Real roundoff_step = 100 * std::numeric_limits<Real>::epsilon() *
                                   std::max({1.0, std::abs(original_coordinate), local_size});
        const Real step = std::max(_relative_step * local_size, roundoff_step);

        coordinate_node(coordinate) = original_coordinate + step;
        amg.computeNodalGeometry();
        const Point plus_normal = amg.getNodalNormals(*representative_elem)[normal_node_index];

        coordinate_node(coordinate) = original_coordinate - step;
        amg.computeNodalGeometry();
        const Point minus_normal = amg.getNodalNormals(*representative_elem)[normal_node_index];

        coordinate_node(coordinate) = original_coordinate;
        amg.computeNodalGeometry();

        const Point fd_derivative = (plus_normal - minus_normal) / (2.0 * step);
        const Real error = (fd_derivative - analytic_derivative).norm();
        const Real reference = std::max(fd_derivative.norm(), analytic_derivative.norm());
        const Real tolerance = _absolute_tolerance / local_size + _relative_tolerance * reference;
        if (error > tolerance)
          mooseError("Mortar nodal-normal sensitivity finite-difference check failed for ",
                     libMesh::Utility::enum_to_string<ElemType>(representative_elem->type()),
                     " normal node ",
                     normal_node_id,
                     ", coordinate node ",
                     candidate_node_id,
                     ", coordinate ",
                     coordinate,
                     ": error = ",
                     error,
                     ", tolerance = ",
                     tolerance,
                     ", analytic derivative = ",
                     analytic_derivative,
                     ", FD derivative = ",
                     fd_derivative,
                     ".");

        ++tested_coordinate_perturbations;
      }

    ++tested_nodes;
  }

  if (!original_node_locations.empty())
  {
    for (const auto & [node, original_location] : original_node_locations)
      for (const auto component : make_range(LIBMESH_DIM))
        (*node)(component) = original_location(component);
    amg.computeNodalGeometry();
  }

  for (const auto elem_type : tested_elem_types)
    _console << "Verified mortar nodal-normal coordinate sensitivities for "
             << libMesh::Utility::enum_to_string<ElemType>(elem_type) << " on " << tested_nodes
             << " secondary nodes using " << tested_coordinate_perturbations
             << " coordinate perturbations" << std::endl;
}
