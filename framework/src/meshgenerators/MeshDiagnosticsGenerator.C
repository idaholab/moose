//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDiagnosticsGenerator.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "MeshCoarseningUtils.h"
#include "MeshBaseDiagnosticsUtils.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/fe.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/face_tri3.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_quad4.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/enum_point_locator_type.h"

registerMooseObject("MooseApp", MeshDiagnosticsGenerator);

InputParameters
MeshDiagnosticsGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to diagnose");
  params.addClassDescription("Runs a series of diagnostics on the mesh to detect potential issues "
                             "such as unsupported features");

  // Options for the output level
  MooseEnum chk_option("NO_CHECK INFO WARNING ERROR", "NO_CHECK");

  params.addParam<MooseEnum>(
      "examine_sidesets_orientation",
      chk_option,
      "whether to check that sidesets are consistently oriented using neighbor subdomains. If a "
      "sideset is inconsistently oriented within a subdomain, this will not be detected");
  params.addParam<MooseEnum>(
      "check_for_watertight_sidesets",
      chk_option,
      "whether to check for external sides that are not assigned to any sidesets");
  params.addParam<MooseEnum>(
      "check_for_watertight_nodesets",
      chk_option,
      "whether to check for external nodes that are not assigned to any nodeset");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries_to_check",
      {},
      "Names boundaries that should form a watertight envelope around the mesh. Defaults to all "
      "the boundaries combined.");
  params.addParam<MooseEnum>(
      "examine_element_volumes", chk_option, "whether to examine volume of the elements");
  params.addParam<Real>("minimum_element_volumes", 1e-16, "minimum size for element volume");
  params.addParam<Real>("maximum_element_volumes", 1e16, "Maximum size for element volume");

  params.addParam<MooseEnum>("examine_element_types",
                             chk_option,
                             "whether to look for multiple element types in the same sub-domain");
  params.addParam<MooseEnum>(
      "examine_element_overlap", chk_option, "whether to find overlapping elements");
  params.addParam<MooseEnum>(
      "examine_nonplanar_sides", chk_option, "whether to check element sides are planar");
  params.addParam<MooseEnum>("examine_non_conformality",
                             chk_option,
                             "whether to examine the conformality of elements in the mesh");
  params.addParam<MooseEnum>("examine_non_matching_edges",
                             chk_option,
                             "Whether to check if there are any intersecting edges");
  params.addParam<Real>("intersection_tol", TOLERANCE, "tolerence for intersecting edges");
  params.addParam<Real>("nonconformal_tol", TOLERANCE, "tolerance for element non-conformality");
  params.addParam<MooseEnum>(
      "search_for_adaptivity_nonconformality",
      chk_option,
      "whether to check for non-conformality arising from adaptive mesh refinement");
  params.addParam<MooseEnum>("check_local_jacobian",
                             chk_option,
                             "whether to check the local Jacobian for negative values");
  params.addParam<unsigned int>(
      "log_length_limit",
      10,
      "How many problematic element/nodes/sides/etc are explicitly reported on by each check");
  return params;
}

MeshDiagnosticsGenerator::MeshDiagnosticsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _check_sidesets_orientation(getParam<MooseEnum>("examine_sidesets_orientation")),
    _check_watertight_sidesets(getParam<MooseEnum>("check_for_watertight_sidesets")),
    _check_watertight_nodesets(getParam<MooseEnum>("check_for_watertight_nodesets")),
    _watertight_boundary_names(getParam<std::vector<BoundaryName>>("boundaries_to_check")),
    _check_element_volumes(getParam<MooseEnum>("examine_element_volumes")),
    _min_volume(getParam<Real>("minimum_element_volumes")),
    _max_volume(getParam<Real>("maximum_element_volumes")),
    _check_element_types(getParam<MooseEnum>("examine_element_types")),
    _check_element_overlap(getParam<MooseEnum>("examine_element_overlap")),
    _check_non_planar_sides(getParam<MooseEnum>("examine_nonplanar_sides")),
    _check_non_conformal_mesh(getParam<MooseEnum>("examine_non_conformality")),
    _non_conformality_tol(getParam<Real>("nonconformal_tol")),
    _check_non_matching_edges(getParam<MooseEnum>("examine_non_matching_edges")),
    _non_matching_edge_tol(getParam<Real>("intersection_tol")),
    _check_adaptivity_non_conformality(
        getParam<MooseEnum>("search_for_adaptivity_nonconformality")),
    _check_local_jacobian(getParam<MooseEnum>("check_local_jacobian")),
    _num_outputs(getParam<unsigned int>("log_length_limit"))
{
  // Check that no secondary parameters have been passed with the main check disabled
  if ((isParamSetByUser("minimum_element_volumes") ||
       isParamSetByUser("maximum_element_volumes")) &&
      _check_element_volumes == "NO_CHECK")
    paramError("examine_element_volumes",
               "You must set this parameter to true to trigger element size checks");
  if (isParamSetByUser("nonconformal_tol") && _check_non_conformal_mesh == "NO_CHECK")
    paramError("examine_non_conformality",
               "You must set this parameter to true to trigger mesh conformality check");
  if (_check_sidesets_orientation == "NO_CHECK" && _check_watertight_sidesets == "NO_CHECK" &&
      _check_watertight_nodesets == "NO_CHECK" && _check_element_volumes == "NO_CHECK" &&
      _check_element_types == "NO_CHECK" && _check_element_overlap == "NO_CHECK" &&
      _check_non_planar_sides == "NO_CHECK" && _check_non_conformal_mesh == "NO_CHECK" &&
      _check_adaptivity_non_conformality == "NO_CHECK" && _check_local_jacobian == "NO_CHECK" &&
      _check_non_matching_edges == "NO_CHECK")
    mooseError("You need to turn on at least one diagnostic. Did you misspell a parameter?");
}

std::unique_ptr<MeshBase>
MeshDiagnosticsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Most of the checks assume we have the full mesh
  if (!mesh->is_serial())
    mooseError("Only serialized meshes are supported");

  // We prepare for use at the beginning to facilitate diagnosis
  // This deliberately does not trust the mesh to know whether it's already prepared or not
  mesh->prepare_for_use();

  // check that specified boundary is valid, convert BoundaryNames to BoundaryIDs, and sort
  for (const auto & boundary_name : _watertight_boundary_names)
  {
    if (!MooseMeshUtils::hasBoundaryName(*mesh, boundary_name))
      mooseError("User specified boundary_to_check \'", boundary_name, "\' does not exist");
  }
  _watertight_boundaries = MooseMeshUtils::getBoundaryIDs(*mesh, _watertight_boundary_names, false);
  std::sort(_watertight_boundaries.begin(), _watertight_boundaries.end());

  if (_check_sidesets_orientation != "NO_CHECK")
    checkSidesetsOrientation(mesh);

  if (_check_watertight_sidesets != "NO_CHECK")
    checkWatertightSidesets(mesh);

  if (_check_watertight_nodesets != "NO_CHECK")
    checkWatertightNodesets(mesh);

  if (_check_element_volumes != "NO_CHECK")
    checkElementVolumes(mesh);

  if (_check_element_types != "NO_CHECK")
    checkElementTypes(mesh);

  if (_check_element_overlap != "NO_CHECK")
    checkElementOverlap(mesh);

  if (_check_non_planar_sides != "NO_CHECK")
    checkNonPlanarSides(mesh);

  if (_check_non_conformal_mesh != "NO_CHECK")
    checkNonConformalMesh(mesh);

  if (_check_adaptivity_non_conformality != "NO_CHECK")
    checkNonConformalMeshFromAdaptivity(mesh);

  if (_check_local_jacobian != "NO_CHECK")
    checkLocalJacobians(mesh);

  if (_check_non_matching_edges != "NO_CHECK")
    checkNonMatchingEdges(mesh);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
MeshDiagnosticsGenerator::checkSidesetsOrientation(const std::unique_ptr<MeshBase> & mesh) const
{
  auto & boundary_info = mesh->get_boundary_info();
  auto side_tuples = boundary_info.build_side_list();

  for (const auto bid : boundary_info.get_boundary_ids())
  {
    // This check only looks at subdomains on both sides of the sideset
    // it wont pick up if the sideset is changing orientations while inside of a subdomain
    std::set<std::pair<subdomain_id_type, subdomain_id_type>> block_neighbors;
    for (const auto index : index_range(side_tuples))
    {
      if (std::get<2>(side_tuples[index]) != bid)
        continue;
      const auto elem_ptr = mesh->elem_ptr(std::get<0>(side_tuples[index]));
      if (elem_ptr->neighbor_ptr(std::get<1>(side_tuples[index])))
        block_neighbors.insert(std::make_pair(
            elem_ptr->subdomain_id(),
            elem_ptr->neighbor_ptr(std::get<1>(side_tuples[index]))->subdomain_id()));
    }

    // Check that there is no flipped pair
    std::set<std::pair<subdomain_id_type, subdomain_id_type>> flipped_pairs;
    for (const auto & block_pair_1 : block_neighbors)
      for (const auto & block_pair_2 : block_neighbors)
        if (block_pair_1 != block_pair_2)
          if (block_pair_1.first == block_pair_2.second &&
              block_pair_1.second == block_pair_2.first)
            flipped_pairs.insert(block_pair_1);

    std::string message;
    const std::string sideset_full_name =
        boundary_info.sideset_name(bid) + " (" + std::to_string(bid) + ")";
    if (!flipped_pairs.empty())
    {
      std::string block_pairs_string = "";
      for (const auto & pair : flipped_pairs)
        block_pairs_string +=
            " [" + mesh->subdomain_name(pair.first) + " (" + std::to_string(pair.first) + "), " +
            mesh->subdomain_name(pair.second) + " (" + std::to_string(pair.second) + ")]";
      message = "Inconsistent orientation of sideset " + sideset_full_name +
                " with regards to subdomain pairs" + block_pairs_string;
    }
    else
      message = "Sideset " + sideset_full_name +
                " is consistently oriented with regards to the blocks it neighbors";

    diagnosticsLog(message, _check_sidesets_orientation, flipped_pairs.size());

    // Now check that there is no sideset radically flipping from one side's normal to another
    // side next to it, in the same sideset
    // We'll consider pi / 2 to be the most steep angle we'll pass
    unsigned int num_normals_flipping = 0;
    Real steepest_side_angles = 1;
    for (const auto & [elem_id, side_id, side_bid] : side_tuples)
    {
      if (side_bid != bid)
        continue;
      const auto & elem_ptr = mesh->elem_ptr(elem_id);

      // Get side normal
      const std::unique_ptr<const Elem> face = elem_ptr->build_side_ptr(side_id);
      std::unique_ptr<libMesh::FEBase> fe(
          libMesh::FEBase::build(elem_ptr->dim(), libMesh::FEType(elem_ptr->default_order())));
      libMesh::QGauss qface(elem_ptr->dim() - 1, CONSTANT);
      fe->attach_quadrature_rule(&qface);
      const auto & normals = fe->get_normals();
      fe->reinit(elem_ptr, side_id);
      mooseAssert(normals.size() == 1, "We expected only one normal here");
      const auto & side_normal = normals[0];

      // Compare to the sideset normals of neighbor sides in that sideset
      for (const auto neighbor : elem_ptr->neighbor_ptr_range())
        if (neighbor)
          for (const auto neigh_side_index : neighbor->side_index_range())
          {
            // Check that the neighbor side is also in the sideset being examined
            if (!boundary_info.has_boundary_id(neighbor, neigh_side_index, bid))
              continue;

            // We re-init everything for the neighbor in case it's a different dimension
            std::unique_ptr<libMesh::FEBase> fe_neighbor(libMesh::FEBase::build(
                neighbor->dim(), libMesh::FEType(neighbor->default_order())));
            libMesh::QGauss qface(neighbor->dim() - 1, CONSTANT);
            fe_neighbor->attach_quadrature_rule(&qface);
            const auto & neigh_normals = fe_neighbor->get_normals();
            fe_neighbor->reinit(neighbor, neigh_side_index);
            mooseAssert(neigh_normals.size() == 1, "We expected only one normal here");
            const auto & neigh_side_normal = neigh_normals[0];

            // Check the angle by computing the dot product
            if (neigh_side_normal * side_normal <= 0)
            {
              num_normals_flipping++;
              steepest_side_angles =
                  std::min(std::acos(neigh_side_normal * side_normal), steepest_side_angles);
              if (num_normals_flipping <= _num_outputs)
                _console << "Side normals changed by more than pi/2 for sideset "
                         << sideset_full_name << " between side " << side_id << " of element "
                         << elem_ptr->id() << " and side " << neigh_side_index
                         << " of neighbor element " << neighbor->id() << std::endl;
              else if (num_normals_flipping == _num_outputs + 1)
                _console << "Maximum output reached for sideset normal flipping check. Silencing "
                            "output from now on"
                         << std::endl;
            }
          }
    }

    if (num_normals_flipping)
      message = "Sideset " + sideset_full_name +
                " has two neighboring sides with a very large angle. Largest angle detected: " +
                std::to_string(steepest_side_angles) + " rad.";
    else
      message = "Sideset " + sideset_full_name +
                " does not appear to have side-to-neighbor-side orientation flips. All neighbor "
                "sides normal differ by less than pi/2";

    diagnosticsLog(message, _check_sidesets_orientation, num_normals_flipping);
  }
}

void
MeshDiagnosticsGenerator::checkWatertightSidesets(const std::unique_ptr<MeshBase> & mesh) const
{
  /*
  Algorithm Overview:
  1) Loop through all elements
  2) For each element loop through all its sides
  3) If it has no neighbors it's an external side
  4) If external check if it's part of a sideset
  */
  if (mesh->mesh_dimension() < 2)
    mooseError("The sideset check only works for 2D and 3D meshes");
  auto & boundary_info = mesh->get_boundary_info();
  boundary_info.build_side_list();
  const auto sideset_map = boundary_info.get_sideset_map();
  unsigned int num_faces_without_sideset = 0;

  for (const auto elem : mesh->active_element_ptr_range())
  {
    for (auto i : elem->side_index_range())
    {
      // Check if side is external
      if (elem->neighbor_ptr(i) == nullptr)
      {
        // If external get the boundary ids associated with this side
        std::vector<boundary_id_type> boundary_ids;
        auto side_range = sideset_map.equal_range(elem);
        for (const auto & itr : as_range(side_range))
          if (itr.second.first == i)
            boundary_ids.push_back(i);
        // get intersection of boundary_ids and _watertight_boundaries
        std::vector<boundary_id_type> intersections =
            findBoundaryOverlap(_watertight_boundaries, boundary_ids);

        bool no_specified_ids = boundary_ids.empty();
        bool specified_ids = !_watertight_boundaries.empty() && intersections.empty();
        std::string message;
        if (mesh->mesh_dimension() == 3)
          message = "Element " + std::to_string(elem->id()) +
                    " contains an external face which has not been assigned to ";
        else
          message = "Element " + std::to_string(elem->id()) +
                    " contains an external edge which has not been assigned to ";
        if (no_specified_ids)
          message = message + "a sideset";
        else if (specified_ids)
          message = message + "one of the specified sidesets";
        if ((no_specified_ids || specified_ids) && num_faces_without_sideset < _num_outputs)
        {
          _console << message << std::endl;
          num_faces_without_sideset++;
        }
      }
    }
  }
  std::string message;
  if (mesh->mesh_dimension() == 3)
    message = "Number of external element faces that have not been assigned to a sideset: " +
              std::to_string(num_faces_without_sideset);
  else
    message = "Number of external element edges that have not been assigned to a sideset: " +
              std::to_string(num_faces_without_sideset);
  diagnosticsLog(message, _check_watertight_sidesets, num_faces_without_sideset);
}

void
MeshDiagnosticsGenerator::checkWatertightNodesets(const std::unique_ptr<MeshBase> & mesh) const
{
  /*
  Diagnostic Overview:
  1) Mesh precheck
  2) Loop through all elements
  3) Loop through all sides of that element
  4) If side is external loop through its nodes
  5) If node is not associated with any nodeset add to list
  6) Print out node id
  */
  if (mesh->mesh_dimension() < 2)
    mooseError("The nodeset check only works for 2D and 3D meshes");
  auto & boundary_info = mesh->get_boundary_info();
  unsigned int num_nodes_without_nodeset = 0;
  std::set<dof_id_type> checked_nodes_id;

  for (const auto elem : mesh->active_element_ptr_range())
  {
    for (const auto i : elem->side_index_range())
    {
      // Check if side is external
      if (elem->neighbor_ptr(i) == nullptr)
      {
        // Side is external, now check nodes
        auto side = elem->side_ptr(i);
        const auto & node_list = side->get_nodes();
        for (unsigned int j = 0; j < side->n_nodes(); j++)
        {
          const auto node = node_list[j];
          if (checked_nodes_id.count(node->id()))
            continue;
          // get vector of node's boundaries (in most cases it will only have one)
          std::vector<boundary_id_type> boundary_ids;
          boundary_info.boundary_ids(node, boundary_ids);
          std::vector<boundary_id_type> intersection =
              findBoundaryOverlap(_watertight_boundaries, boundary_ids);

          bool no_specified_ids = boundary_info.n_boundary_ids(node) == 0;
          bool specified_ids = !_watertight_boundaries.empty() && intersection.empty();
          std::string message =
              "Node " + std::to_string(node->id()) +
              " is on an external boundary of the mesh, but has not been assigned to ";
          if (no_specified_ids)
            message = message + "a nodeset";
          else if (specified_ids)
            message = message + "one of the specified nodesets";
          if ((no_specified_ids || specified_ids) && num_nodes_without_nodeset < _num_outputs)
          {
            checked_nodes_id.insert(node->id());
            num_nodes_without_nodeset++;
            _console << message << std::endl;
          }
        }
      }
    }
  }
  std::string message;
  message = "Number of external nodes that have not been assigned to a nodeset: " +
            std::to_string(num_nodes_without_nodeset);
  diagnosticsLog(message, _check_watertight_nodesets, num_nodes_without_nodeset);
}

std::vector<boundary_id_type>
MeshDiagnosticsGenerator::findBoundaryOverlap(
    const std::vector<boundary_id_type> & watertight_boundaries,
    std::vector<boundary_id_type> & boundary_ids) const
{
  // Only the boundary_ids vector is sorted here. watertight_boundaries has to be sorted beforehand
  // Returns their intersection (elements that they share)
  std::sort(boundary_ids.begin(), boundary_ids.end());
  std::vector<boundary_id_type> boundary_intersection;
  std::set_intersection(watertight_boundaries.begin(),
                        watertight_boundaries.end(),
                        boundary_ids.begin(),
                        boundary_ids.end(),
                        std::back_inserter(boundary_intersection));
  return boundary_intersection;
}

void
MeshDiagnosticsGenerator::checkElementVolumes(const std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_tiny_elems = 0;
  unsigned int num_negative_elems = 0;
  unsigned int num_big_elems = 0;
  // loop elements within the mesh (assumes replicated)
  for (auto & elem : mesh->active_element_ptr_range())
  {
    if (elem->volume() <= _min_volume)
    {
      if (num_tiny_elems < _num_outputs)
        _console << "Element with volume below threshold detected : \n"
                 << "id " << elem->id() << " near point " << elem->vertex_average() << std::endl;
      else if (num_tiny_elems == _num_outputs)
        _console << "Maximum output reached, log is silenced" << std::endl;
      num_tiny_elems++;
    }
    if (elem->volume() >= _max_volume)
    {
      if (num_big_elems < _num_outputs)
        _console << "Element with volume above threshold detected : \n"
                 << elem->get_info() << std::endl;
      else if (num_big_elems == _num_outputs)
        _console << "Maximum output reached, log is silenced" << std::endl;
      num_big_elems++;
    }
  }
  diagnosticsLog("Number of elements below prescribed minimum volume : " +
                     std::to_string(num_tiny_elems),
                 _check_element_volumes,
                 num_tiny_elems);
  diagnosticsLog("Number of elements with negative volume : " + std::to_string(num_negative_elems),
                 _check_element_volumes,
                 num_negative_elems);
  diagnosticsLog("Number of elements above prescribed maximum volume : " +
                     std::to_string(num_big_elems),
                 _check_element_volumes,
                 num_big_elems);
}

void
MeshDiagnosticsGenerator::checkElementTypes(const std::unique_ptr<MeshBase> & mesh) const
{
  std::set<subdomain_id_type> ids;
  mesh->subdomain_ids(ids);
  // loop on sub-domain
  for (auto & id : ids)
  {
    // ElemType defines an enum for geometric element types
    std::set<ElemType> types;
    // loop on elements within this sub-domain
    for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
      types.insert(elem->type());

    std::string elem_type_names = "";
    for (auto & elem_type : types)
      elem_type_names += " " + Moose::stringify(elem_type);

    _console << "Element type in subdomain " + mesh->subdomain_name(id) + " (" +
                    std::to_string(id) + ") :" + elem_type_names
             << std::endl;
    if (types.size() > 1)
      diagnosticsLog("Two different element types in subdomain " + std::to_string(id),
                     _check_element_types,
                     true);
  }
}

void
MeshDiagnosticsGenerator::checkElementOverlap(const std::unique_ptr<MeshBase> & mesh) const
{
  {
    unsigned int num_elem_overlaps = 0;
    auto pl = mesh->sub_point_locator();
    // loop on nodes, assumed replicated mesh
    for (auto & node : mesh->node_ptr_range())
    {
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      for (auto & elem : elements)
      {
        if (!elem->contains_point(*node))
          continue;

        // not overlapping inside the element if part of its nodes
        bool found = false;
        for (auto & elem_node : elem->node_ref_range())
          if (*node == elem_node)
          {
            found = true;
            break;
          }
        // not overlapping inside the element if right on its side
        bool on_a_side = false;
        for (const auto & elem_side_index : elem->side_index_range())
          if (elem->side_ptr(elem_side_index)->contains_point(*node, _non_conformality_tol))
            on_a_side = true;
        if (!found && !on_a_side)
        {
          num_elem_overlaps++;
          if (num_elem_overlaps < _num_outputs)
            _console << "Element overlap detected at : " << *node << std::endl;
          else if (num_elem_overlaps == _num_outputs)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }

    diagnosticsLog("Number of elements overlapping (node-based heuristics): " +
                       Moose::stringify(num_elem_overlaps),
                   _check_element_overlap,
                   num_elem_overlaps);
    num_elem_overlaps = 0;

    // loop on all elements in mesh: assumes a replicated mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      // find all the elements around the centroid of this element
      std::set<const Elem *> overlaps;
      (*pl)(elem->vertex_average(), overlaps);

      if (overlaps.size() > 1)
      {
        num_elem_overlaps++;
        if (num_elem_overlaps < _num_outputs)
          _console << "Element overlap detected with element : " << elem->id() << " near point "
                   << elem->vertex_average() << std::endl;
        else if (num_elem_overlaps == _num_outputs)
          _console << "Maximum output reached, log is silenced" << std::endl;
      }
    }
    diagnosticsLog("Number of elements overlapping (centroid-based heuristics): " +
                       Moose::stringify(num_elem_overlaps),
                   _check_element_overlap,
                   num_elem_overlaps);
  }
}

void
MeshDiagnosticsGenerator::checkNonPlanarSides(const std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int sides_non_planar = 0;
  // loop on all elements in mesh: assumes a replicated mesh
  for (auto & elem : mesh->active_element_ptr_range())
  {
    for (auto i : make_range(elem->n_sides()))
    {
      auto side = elem->side_ptr(i);
      std::vector<const Point *> nodes;
      for (auto & node : side->node_ref_range())
        nodes.emplace_back(&node);

      if (nodes.size() <= 3)
        continue;
      // First vector of the base
      const RealVectorValue v1 = *nodes[0] - *nodes[1];

      // Find another node so that we can form a basis. It should just be node 0, 1, 2
      // to form two independent vectors, but degenerate elements can make them aligned
      bool aligned = true;
      unsigned int third_node_index = 2;
      RealVectorValue v2;
      while (aligned && third_node_index < nodes.size())
      {
        v2 = *nodes[0] - *nodes[third_node_index++];
        aligned = MooseUtils::absoluteFuzzyEqual(v1 * v2 - v1.norm() * v2.norm(), 0);
      }

      // Degenerate element, could not find a third node that is not aligned
      if (aligned)
        continue;

      bool found_non_planar = false;

      for (auto node_offset : make_range(nodes.size() - 3))
      {
        RealVectorValue v3 = *nodes[0] - *nodes[node_offset + 3];
        bool planar = MooseUtils::absoluteFuzzyEqual(v2.cross(v1) * v3, 0);
        if (!planar)
          found_non_planar = true;
      }

      if (found_non_planar)
      {
        sides_non_planar++;
        if (sides_non_planar < _num_outputs)
          _console << "Nonplanar side detected for side " << i
                   << " of element :" << elem->get_info() << std::endl;
        else if (sides_non_planar == _num_outputs)
          _console << "Maximum output reached, log is silenced" << std::endl;
      }
    }
  }
  diagnosticsLog("Number of non-planar element sides detected: " +
                     Moose::stringify(sides_non_planar),
                 _check_non_planar_sides,
                 sides_non_planar);
}

void
MeshDiagnosticsGenerator::checkNonConformalMesh(const std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_nonconformal_nodes = 0;
  MeshBaseDiagnosticsUtils::checkNonConformalMesh(
      mesh, _console, _num_outputs, _non_conformality_tol, num_nonconformal_nodes);
  diagnosticsLog("Number of non-conformal nodes: " + Moose::stringify(num_nonconformal_nodes),
                 _check_non_conformal_mesh,
                 num_nonconformal_nodes);
}

void
MeshDiagnosticsGenerator::checkNonConformalMeshFromAdaptivity(
    const std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_likely_AMR_created_nonconformality = 0;
  auto pl = mesh->sub_point_locator();
  pl->set_close_to_point_tol(_non_conformality_tol);

  // We have to make a copy because adding the new parent element to the mesh
  // will modify the mesh for the analysis of the next nodes
  // Make a copy of the mesh, add this element
  auto mesh_copy = mesh->clone();
  libMesh::MeshRefinement mesh_refiner(*mesh_copy);

  // loop on nodes, assumes a replicated mesh
  for (auto & node : mesh->node_ptr_range())
  {
    // find all the elements around this node
    std::set<const Elem *> elements_around;
    (*pl)(*node, elements_around);

    // Keep track of the refined elements and the coarse element
    std::set<const Elem *> fine_elements;
    std::set<const Elem *> coarse_elements;

    // loop through the set of elements near this node
    for (auto elem : elements_around)
    {
      // If the node is not part of this element's nodes, it is a
      // case of non-conformality
      bool node_on_elem = false;

      // non-vertex nodes are not cause for the kind of non-conformality we are looking for
      if (!elem->is_vertex(elem->get_node_index(node)))
        continue;

      if (elem->get_node_index(node) != libMesh::invalid_uint)
        node_on_elem = true;

      // Keep track of all the elements this node is a part of. They are potentially the
      // 'fine' (refined) elements next to a coarser element
      if (node_on_elem)
        fine_elements.insert(elem);
      // Else, the node is not part of the element considered, so if the element had been part
      // of an AMR-created non-conformality, this element is on the coarse side
      if (!node_on_elem)
        coarse_elements.insert(elem);
    }

    // all the elements around contained the node as one of their nodes
    // if the coarse and refined sides are not stitched together, this check can fail,
    // as nodes that are physically near one element are not part of it because of the lack of
    // stitching (overlapping nodes)
    if (fine_elements.size() == elements_around.size())
      continue;

    if (fine_elements.empty())
      continue;

    // Depending on the type of element, we already know the number of elements we expect
    // to be part of this set of likely refined candidates for a given non-conformal node to
    // examine. We can only decide if it was born out of AMR if it's the center node of the face
    // of a coarse element near refined elements
    const auto elem_type = (*fine_elements.begin())->type();
    if ((elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9) &&
        fine_elements.size() != 2)
      continue;
    else if ((elem_type == HEX8 || elem_type == HEX20 || elem_type == HEX27) &&
             fine_elements.size() != 4)
      continue;
    else if ((elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7) &&
             fine_elements.size() != 3)
      continue;
    else if ((elem_type == TET4 || elem_type == TET10 || elem_type == TET14) &&
             (fine_elements.size() % 4 != 0))
      continue;

    // only one coarse element in front of refined elements except for tets. Whatever we're
    // looking at is not the interface between coarse and refined elements
    // Tets are split on their edges (rather than the middle of a face) so there could be any
    // number of coarse elements in front of the node non-conformality created by refinement
    if (elem_type != TET4 && elem_type != TET10 && elem_type != TET14 && coarse_elements.size() > 1)
      continue;

    // There exists non-conformality, the node should have been a node of all the elements
    // that are close enough to the node, and it is not

    // Nodes of the tentative parent element
    std::vector<const Node *> tentative_coarse_nodes;

    // For quads and hexes, there is one (quad) or four (hexes) sides that are tied to this node
    // at the non-conformal interface between the refined elements and a coarse element
    if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9 || elem_type == HEX8 ||
        elem_type == HEX20 || elem_type == HEX27)
    {
      const auto elem = *fine_elements.begin();

      // Find which sides (of the elements) the node considered is part of
      std::vector<Elem *> node_on_sides;
      unsigned int side_inside_parent = std::numeric_limits<unsigned int>::max();
      for (auto i : make_range(elem->n_sides()))
      {
        const auto side = elem->side_ptr(i);
        std::vector<const Node *> other_nodes_on_side;
        bool node_on_side = false;
        for (const auto & elem_node : side->node_ref_range())
        {
          if (*node == elem_node)
            node_on_side = true;
          else
            other_nodes_on_side.emplace_back(&elem_node);
        }
        // node is on the side, but is it the side that goes away from the coarse element?
        if (node_on_side)
        {
          // if all the other nodes on this side are in one of the other potentially refined
          // elements, it's one of the side(s) (4 sides in a 3D hex for example) inside the
          // parent
          bool all_side_nodes_are_shared = true;
          for (const auto & other_node : other_nodes_on_side)
          {
            bool shared_with_a_fine_elem = false;
            for (const auto & other_elem : fine_elements)
              if (other_elem != elem &&
                  other_elem->get_node_index(other_node) != libMesh::invalid_uint)
                shared_with_a_fine_elem = true;

            if (!shared_with_a_fine_elem)
              all_side_nodes_are_shared = false;
          }
          if (all_side_nodes_are_shared)
          {
            side_inside_parent = i;
            // We stop examining sides, it does not matter which side we pick inside the parent
            break;
          }
        }
      }
      if (side_inside_parent == std::numeric_limits<unsigned int>::max())
        continue;

      // Gather the other potential elements in the refined element:
      // they are point neighbors of the node that is shared between all the elements flagged
      // for the non-conformality
      // Find shared node
      const auto interior_side = elem->side_ptr(side_inside_parent);
      const Node * interior_node = nullptr;
      for (const auto & other_node : interior_side->node_ref_range())
      {
        if (other_node == *node)
          continue;
        bool in_all_node_neighbor_elements = true;
        for (auto other_elem : fine_elements)
        {
          if (other_elem->get_node_index(&other_node) == libMesh::invalid_uint)
            in_all_node_neighbor_elements = false;
        }
        if (in_all_node_neighbor_elements)
        {
          interior_node = &other_node;
          break;
        }
      }
      // Did not find interior node. Probably not AMR
      if (!interior_node)
        continue;

      // Add point neighbors of interior node to list of potentially refined elements
      std::set<const Elem *> all_elements;
      elem->find_point_neighbors(*interior_node, all_elements);

      if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9)
      {
        const auto success = MeshCoarseningUtils::getFineElementsFromInteriorNode(
            *interior_node, *node, *elem, tentative_coarse_nodes, fine_elements);
        if (!success)
          continue;
      }
      // For hexes we first look at the fine-neighbors of the non-conformality
      // then the fine elements neighbors of the center 'node' of the potential parent
      else
      {
        // Get the coarse neighbor side to be able to recognize nodes that should become part of
        // the coarse parent
        const auto & coarse_elem = *coarse_elements.begin();
        unsigned short coarse_side_i = 0;
        for (const auto & coarse_side_index : coarse_elem->side_index_range())
        {
          const auto coarse_side_ptr = coarse_elem->side_ptr(coarse_side_index);
          // The side of interest is the side that contains the non-conformality
          if (!coarse_side_ptr->close_to_point(*node, 10 * _non_conformality_tol))
            continue;
          else
          {
            coarse_side_i = coarse_side_index;
            break;
          }
        }
        const auto coarse_side = coarse_elem->side_ptr(coarse_side_i);

        // We did not find the side of the coarse neighbor near the refined elements
        // Try again at another node
        if (!coarse_side)
          continue;

        // We cant directly use the coarse neighbor nodes
        // - The user might be passing a disjoint mesh
        // - There could two levels of refinement separating the coarse neighbor and its refined
        // counterparts
        // We use the fine element nodes
        unsigned int i = 0;
        tentative_coarse_nodes.resize(4);
        for (const auto & elem_1 : fine_elements)
          for (const auto & coarse_node : elem_1->node_ref_range())
          {
            bool node_shared = false;
            for (const auto & elem_2 : fine_elements)
            {
              if (elem_2 != elem_1)
                if (elem_2->get_node_index(&coarse_node) != libMesh::invalid_uint)
                  node_shared = true;
            }
            // A node for the coarse parent will appear in only one fine neighbor (not shared)
            // and will lay on the side of the coarse neighbor
            if (!node_shared && coarse_side->close_to_point(coarse_node, _non_conformality_tol))
              tentative_coarse_nodes[i++] = &coarse_node;
            mooseAssert(i <= 5, "We went too far in this index");
          }

        // We did not find 4 coarse nodes. Mesh might be disjoint and the coarse element does not
        // contain the fine elements nodes we found
        if (i != 4)
          continue;

        // Need to order these nodes to form a valid quad / base of an hex
        // We go around the axis formed by the node and the interior node
        Point axis = *interior_node - *node;
        const auto start_circle = elem->vertex_average();
        MeshCoarseningUtils::reorderNodes(
            tentative_coarse_nodes, *interior_node, start_circle, axis);
        tentative_coarse_nodes.resize(8);

        // Use the neighbors of the fine elements that contain these nodes to get the vertex
        // nodes
        for (const auto & elem : fine_elements)
        {
          // Find the index of the coarse node for the starting element
          unsigned int node_index = 0;
          for (const auto & coarse_node : tentative_coarse_nodes)
          {
            if (elem->get_node_index(coarse_node) != libMesh::invalid_uint)
              break;
            node_index++;
          }

          // Get the neighbor element that is part of the fine elements to coarsen together
          for (const auto & neighbor : elem->neighbor_ptr_range())
            if (all_elements.count(neighbor) && !fine_elements.count(neighbor))
            {
              // Find the coarse node for the neighbor
              const Node * coarse_elem_node = nullptr;
              for (const auto & fine_node : neighbor->node_ref_range())
              {
                if (!neighbor->is_vertex(neighbor->get_node_index(&fine_node)))
                  continue;
                bool node_shared = false;
                for (const auto & elem_2 : all_elements)
                  if (elem_2 != neighbor &&
                      elem_2->get_node_index(&fine_node) != libMesh::invalid_uint)
                    node_shared = true;
                if (!node_shared)
                {
                  coarse_elem_node = &fine_node;
                  break;
                }
              }
              // Insert the coarse node at the right place
              tentative_coarse_nodes[node_index + 4] = coarse_elem_node;
              mooseAssert(node_index + 4 < tentative_coarse_nodes.size(), "Indexed too far");
              mooseAssert(coarse_elem_node, "Did not find last coarse element node");
            }
        }
      }

      // No need to separate fine elements near the non-conformal node and away from it
      fine_elements = all_elements;
    }
    // For TRI elements, we use the fine triangle element at the center of the potential
    // coarse triangle element
    else if (elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7)
    {
      // Find the center element
      // It's the only element that shares a side with both of the other elements near the node
      // considered
      const Elem * center_elem = nullptr;
      for (const auto refined_elem_1 : fine_elements)
      {
        unsigned int num_neighbors = 0;
        for (const auto refined_elem_2 : fine_elements)
        {
          if (refined_elem_1 == refined_elem_2)
            continue;
          if (refined_elem_1->has_neighbor(refined_elem_2))
            num_neighbors++;
        }
        if (num_neighbors >= 2)
          center_elem = refined_elem_1;
      }
      // Did not find the center fine element, probably not AMR
      if (!center_elem)
        continue;
      // Now get the tentative coarse element nodes
      for (const auto refined_elem : fine_elements)
      {
        if (refined_elem == center_elem)
          continue;
        for (const auto & other_node : refined_elem->node_ref_range())
          if (center_elem->get_node_index(&other_node) == libMesh::invalid_uint &&
              refined_elem->is_vertex(refined_elem->get_node_index(&other_node)))
            tentative_coarse_nodes.push_back(&other_node);
      }

      // Get the final tentative new coarse element node, on the other side of the center
      // element from the non-conformality
      unsigned int center_side_opposite_node = std::numeric_limits<unsigned int>::max();
      for (auto side_index : center_elem->side_index_range())
        if (center_elem->side_ptr(side_index)->get_node_index(node) == libMesh::invalid_uint)
          center_side_opposite_node = side_index;
      const auto neighbor_on_other_side_of_opposite_center_side =
          center_elem->neighbor_ptr(center_side_opposite_node);

      // Element is on a boundary, cannot form a coarse element
      if (!neighbor_on_other_side_of_opposite_center_side)
        continue;

      fine_elements.insert(neighbor_on_other_side_of_opposite_center_side);
      for (const auto & tri_node : neighbor_on_other_side_of_opposite_center_side->node_ref_range())
        if (neighbor_on_other_side_of_opposite_center_side->is_vertex(
                neighbor_on_other_side_of_opposite_center_side->get_node_index(&tri_node)) &&
            center_elem->side_ptr(center_side_opposite_node)->get_node_index(&tri_node) ==
                libMesh::invalid_uint)
          tentative_coarse_nodes.push_back(&tri_node);

      mooseAssert(center_side_opposite_node != std::numeric_limits<unsigned int>::max(),
                  "Did not find the side opposite the non-conformality");
      mooseAssert(tentative_coarse_nodes.size() == 3,
                  "We are forming a coarsened triangle element");
    }
    // For TET elements, it's very different because the non-conformality does not happen inside
    // of a face, but on an edge of one or more coarse elements
    else if (elem_type == TET4 || elem_type == TET10 || elem_type == TET14)
    {
      // There are 4 tets on the tips of the coarsened tet and 4 tets inside
      // let's identify all of them
      std::set<const Elem *> tips_tets;
      std::set<const Elem *> inside_tets;

      // pick a coarse element and work with its fine neighbors
      const Elem * coarse_elem = nullptr;
      std::set<const Elem *> fine_tets;
      for (auto & coarse_one : coarse_elements)
      {
        for (const auto & elem : fine_elements)
          // for two levels of refinement across, this is not working
          // we would need a "has_face_embedded_in_this_other_ones_face" routine
          if (elem->has_neighbor(coarse_one))
            fine_tets.insert(elem);

        if (fine_tets.size())
        {
          coarse_elem = coarse_one;
          break;
        }
      }
      // There's no coarse element neighbor to a group of finer tets, not AMR
      if (!coarse_elem)
        continue;

      // There is one last point neighbor of the node that is sandwiched between two neighbors
      for (const auto & elem : fine_elements)
      {
        int num_face_neighbors = 0;
        for (const auto & tet : fine_tets)
          if (tet->has_neighbor(elem))
            num_face_neighbors++;
        if (num_face_neighbors == 2)
        {
          fine_tets.insert(elem);
          break;
        }
      }

      // There should be two other nodes with non-conformality near this coarse element
      // Find both, as they will be nodes of the rest of the elements to add to the potential
      // fine tet list. They are shared by two of the fine tets we have already found
      std::set<const Node *> other_nodes;
      for (const auto & tet_1 : fine_tets)
      {
        for (const auto & node_1 : tet_1->node_ref_range())
        {
          if (&node_1 == node)
            continue;
          if (!tet_1->is_vertex(tet_1->get_node_index(&node_1)))
            continue;
          for (const auto & tet_2 : fine_tets)
          {
            if (tet_2 == tet_1)
              continue;
            if (tet_2->get_node_index(&node_1) != libMesh::invalid_uint)
              // check that it's near the coarse element as well
              if (coarse_elem->close_to_point(node_1, 10 * _non_conformality_tol))
                other_nodes.insert(&node_1);
          }
        }
      }
      mooseAssert(other_nodes.size() == 2,
                  "Should find only two extra non-conformal nodes near the coarse element");

      // Now we can go towards this tip element next to two non-conformalities
      for (const auto & tet_1 : fine_tets)
      {
        for (const auto & neighbor : tet_1->neighbor_ptr_range())
          if (neighbor->get_node_index(*other_nodes.begin()) != libMesh::invalid_uint &&
              neighbor->is_vertex(neighbor->get_node_index(*other_nodes.begin())) &&
              neighbor->get_node_index(*other_nodes.rbegin()) != libMesh::invalid_uint &&
              neighbor->is_vertex(neighbor->get_node_index(*other_nodes.rbegin())))
            fine_tets.insert(neighbor);
      }
      // Now that the element next to the time is in the fine_tets, we can get the tip
      for (const auto & tet_1 : fine_tets)
      {
        for (const auto & neighbor : tet_1->neighbor_ptr_range())
          if (neighbor->get_node_index(*other_nodes.begin()) != libMesh::invalid_uint &&
              neighbor->is_vertex(neighbor->get_node_index(*other_nodes.begin())) &&
              neighbor->get_node_index(*other_nodes.rbegin()) != libMesh::invalid_uint &&
              neighbor->is_vertex(neighbor->get_node_index(*other_nodes.rbegin())))
            fine_tets.insert(neighbor);
      }

      // Get the sandwiched tets between the tets we already found
      for (const auto & tet_1 : fine_tets)
        for (const auto & neighbor : tet_1->neighbor_ptr_range())
          for (const auto & tet_2 : fine_tets)
            if (tet_1 != tet_2 && tet_2->has_neighbor(neighbor) && neighbor != coarse_elem)
              fine_tets.insert(neighbor);

      // tips tests are the only ones to have a node that is shared by no other tet in the group
      for (const auto & tet_1 : fine_tets)
      {
        unsigned int unshared_nodes = 0;
        for (const auto & other_node : tet_1->node_ref_range())
        {
          if (!tet_1->is_vertex(tet_1->get_node_index(&other_node)))
            continue;
          bool node_shared = false;
          for (const auto & tet_2 : fine_tets)
            if (tet_2 != tet_1 && tet_2->get_node_index(&other_node) != libMesh::invalid_uint)
              node_shared = true;
          if (!node_shared)
            unshared_nodes++;
        }
        if (unshared_nodes == 1)
          tips_tets.insert(tet_1);
        else if (unshared_nodes == 0)
          inside_tets.insert(tet_1);
        else
          mooseError("Did not expect a tet to have two unshared vertex nodes here");
      }

      // Finally grab the last tip of the tentative coarse tet. It shares:
      // - 3 nodes with the other tips, only one with each
      // - 1 face with only one tet of the fine tet group
      // - it has a node that no other fine tet shares (the tip node)
      for (const auto & tet : inside_tets)
      {
        for (const auto & neighbor : tet->neighbor_ptr_range())
        {
          // Check that it shares a face with no other potential fine tet
          bool shared_with_another_tet = false;
          for (const auto & tet_2 : fine_tets)
          {
            if (tet_2 == tet)
              continue;
            if (tet_2->has_neighbor(neighbor))
              shared_with_another_tet = true;
          }
          if (shared_with_another_tet)
            continue;

          // Used to count the nodes shared with tip tets. Can only be 1 per tip tet
          std::vector<const Node *> tip_nodes_shared;
          unsigned int unshared_nodes = 0;
          for (const auto & other_node : neighbor->node_ref_range())
          {
            if (!neighbor->is_vertex(neighbor->get_node_index(&other_node)))
              continue;

            // Check for being a node-neighbor of the 3 other tip tets
            for (const auto & tip_tet : tips_tets)
            {
              if (neighbor == tip_tet)
                continue;

              // we could break here but we want to check that no other tip shares that node
              if (tip_tet->get_node_index(&other_node) != libMesh::invalid_uint)
                tip_nodes_shared.push_back(&other_node);
            }
            // Check for having a node shared with no other tet
            bool node_shared = false;
            for (const auto & tet_2 : fine_tets)
              if (tet_2 != neighbor && tet_2->get_node_index(&other_node) != libMesh::invalid_uint)
                node_shared = true;
            if (!node_shared)
              unshared_nodes++;
          }
          if (tip_nodes_shared.size() == 3 && unshared_nodes == 1)
            tips_tets.insert(neighbor);
        }
      }

      // append the missing fine tets (inside the coarse element, away from the node considered)
      // into the fine elements set for the check on "did it refine the tentative coarse tet
      // onto the same fine tets"
      fine_elements.clear();
      for (const auto & elem : tips_tets)
        fine_elements.insert(elem);
      for (const auto & elem : inside_tets)
        fine_elements.insert(elem);

      // get the vertex of the coarse element from the tip tets
      for (const auto & tip : tips_tets)
      {
        for (const auto & node : tip->node_ref_range())
        {
          bool outside = true;

          const auto id = tip->get_node_index(&node);
          if (!tip->is_vertex(id))
            continue;
          for (const auto & tet : inside_tets)
            if (tet->get_node_index(&node) != libMesh::invalid_uint)
              outside = false;
          if (outside)
          {
            tentative_coarse_nodes.push_back(&node);
            // only one tip node per tip tet
            break;
          }
        }
      }

      std::sort(tentative_coarse_nodes.begin(), tentative_coarse_nodes.end());
      tentative_coarse_nodes.erase(
          std::unique(tentative_coarse_nodes.begin(), tentative_coarse_nodes.end()),
          tentative_coarse_nodes.end());

      // The group of fine elements ended up having less or more than 4 tips, so it's clearly
      // not forming a coarse tetrahedral
      if (tentative_coarse_nodes.size() != 4)
        continue;
    }
    else
    {
      mooseInfo("Unsupported element type ",
                elem_type,
                ". Skipping detection for this node and all future nodes near only this "
                "element type");
      continue;
    }

    // Check the fine element types: if not all the same then it's not uniform AMR
    for (auto elem : fine_elements)
      if (elem->type() != elem_type)
        continue;

    // Check the number of coarse element nodes gathered
    for (const auto & check_node : tentative_coarse_nodes)
      if (check_node == nullptr)
        continue;

    // Form a parent, of a low order type as we only have the extreme vertex nodes
    std::unique_ptr<Elem> parent = Elem::build(Elem::first_order_equivalent_type(elem_type));
    auto parent_ptr = mesh_copy->add_elem(parent.release());

    // Set the nodes to the coarse element
    for (auto i : index_range(tentative_coarse_nodes))
      parent_ptr->set_node(i) = mesh_copy->node_ptr(tentative_coarse_nodes[i]->id());

    // Refine this parent
    parent_ptr->set_refinement_flag(Elem::REFINE);
    parent_ptr->refine(mesh_refiner);
    const auto num_children = parent_ptr->n_children();

    // Compare with the original set of elements
    // We already know the child share the exterior node. If they share the same vertex
    // average as the group of unrefined elements we will call this good enough for now
    // For tetrahedral elements we cannot rely on the children all matching as the choice in
    // the diagonal selection can be made differently. We'll just say 4 matching children is
    // good enough for the heuristic
    unsigned int num_children_match = 0;
    for (const auto & child : parent_ptr->child_ref_range())
    {
      for (const auto & potential_children : fine_elements)
        if (MooseUtils::absoluteFuzzyEqual(child.vertex_average()(0),
                                           potential_children->vertex_average()(0),
                                           _non_conformality_tol) &&
            MooseUtils::absoluteFuzzyEqual(child.vertex_average()(1),
                                           potential_children->vertex_average()(1),
                                           _non_conformality_tol) &&
            MooseUtils::absoluteFuzzyEqual(child.vertex_average()(2),
                                           potential_children->vertex_average()(2),
                                           _non_conformality_tol))
        {
          num_children_match++;
          break;
        }
    }

    if (num_children_match == num_children ||
        ((elem_type == TET4 || elem_type == TET10 || elem_type == TET14) &&
         num_children_match == 4))
    {
      num_likely_AMR_created_nonconformality++;
      if (num_likely_AMR_created_nonconformality < _num_outputs)
      {
        _console << "Detected non-conformality likely created by AMR near" << *node
                 << Moose::stringify(elem_type)
                 << " elements that could be merged into a coarse element:" << std::endl;
        for (const auto & elem : fine_elements)
          _console << elem->id() << " ";
        _console << std::endl;
      }
      else if (num_likely_AMR_created_nonconformality == _num_outputs)
        _console << "Maximum log output reached, silencing output" << std::endl;
    }
  }

  diagnosticsLog(
      "Number of non-conformal nodes likely due to mesh refinement detected by heuristic: " +
          Moose::stringify(num_likely_AMR_created_nonconformality),
      _check_adaptivity_non_conformality,
      num_likely_AMR_created_nonconformality);
  pl->unset_close_to_point_tol();
}

void
MeshDiagnosticsGenerator::checkLocalJacobians(const std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_negative_elem_qp_jacobians = 0;
  // Get a high-ish order quadrature
  auto qrule_dimension = mesh->mesh_dimension();
  libMesh::QGauss qrule(qrule_dimension, FIFTH);

  // Use a constant monomial
  const libMesh::FEType fe_type(CONSTANT, libMesh::MONOMIAL);

  // Initialize a basic constant monomial shape function everywhere
  std::unique_ptr<libMesh::FEBase> fe_elem;
  if (mesh->mesh_dimension() == 1)
    fe_elem = std::make_unique<libMesh::FEMonomial<1>>(fe_type);
  if (mesh->mesh_dimension() == 2)
    fe_elem = std::make_unique<libMesh::FEMonomial<2>>(fe_type);
  else
    fe_elem = std::make_unique<libMesh::FEMonomial<3>>(fe_type);

  fe_elem->get_JxW();
  fe_elem->attach_quadrature_rule(&qrule);

  // Check elements (assumes serialized mesh)
  for (const auto & elem : mesh->element_ptr_range())
  {
    // Handle mixed-dimensional meshes
    if (qrule_dimension != elem->dim())
    {
      // Re-initialize a quadrature
      qrule_dimension = elem->dim();
      qrule = libMesh::QGauss(qrule_dimension, FIFTH);

      // Re-initialize a monomial FE
      if (elem->dim() == 1)
        fe_elem = std::make_unique<libMesh::FEMonomial<1>>(fe_type);
      if (elem->dim() == 2)
        fe_elem = std::make_unique<libMesh::FEMonomial<2>>(fe_type);
      else
        fe_elem = std::make_unique<libMesh::FEMonomial<3>>(fe_type);

      fe_elem->get_JxW();
      fe_elem->attach_quadrature_rule(&qrule);
    }

    try
    {
      fe_elem->reinit(elem);
    }
    catch (libMesh::LogicError & e)
    {
      num_negative_elem_qp_jacobians++;
      const auto msg = std::string(e.what());
      if (msg.find("negative Jacobian") != std::string::npos)
      {
        if (num_negative_elem_qp_jacobians < _num_outputs)
          _console << "Negative Jacobian found in element " << elem->id() << " near point "
                   << elem->vertex_average() << std::endl;
        else if (num_negative_elem_qp_jacobians == _num_outputs)
          _console << "Maximum log output reached, silencing output" << std::endl;
      }
      else
        _console << e.what() << std::endl;
    }
  }
  diagnosticsLog("Number of elements with a negative Jacobian: " +
                     Moose::stringify(num_negative_elem_qp_jacobians),
                 _check_local_jacobian,
                 num_negative_elem_qp_jacobians);

  unsigned int num_negative_side_qp_jacobians = 0;
  // Get a high-ish order side quadrature
  auto qrule_side_dimension = mesh->mesh_dimension() - 1;
  libMesh::QGauss qrule_side(qrule_side_dimension, FIFTH);

  // Use the side quadrature now
  fe_elem->attach_quadrature_rule(&qrule_side);

  // Check element sides
  for (const auto & elem : mesh->element_ptr_range())
  {
    // Handle mixed-dimensional meshes
    if (int(qrule_side_dimension) != elem->dim() - 1)
    {
      qrule_side_dimension = elem->dim() - 1;
      qrule_side = libMesh::QGauss(qrule_side_dimension, FIFTH);

      // Re-initialize a side FE
      if (elem->dim() == 1)
        fe_elem = std::make_unique<libMesh::FEMonomial<1>>(fe_type);
      if (elem->dim() == 2)
        fe_elem = std::make_unique<libMesh::FEMonomial<2>>(fe_type);
      else
        fe_elem = std::make_unique<libMesh::FEMonomial<3>>(fe_type);

      fe_elem->get_JxW();
      fe_elem->attach_quadrature_rule(&qrule_side);
    }

    for (const auto & side : elem->side_index_range())
    {
      try
      {
        fe_elem->reinit(elem, side);
      }
      catch (libMesh::LogicError & e)
      {
        const auto msg = std::string(e.what());
        if (msg.find("negative Jacobian") != std::string::npos)
        {
          num_negative_side_qp_jacobians++;
          if (num_negative_side_qp_jacobians < _num_outputs)
            _console << "Negative Jacobian found in side " << side << " of element" << elem->id()
                     << " near point " << elem->vertex_average() << std::endl;
          else if (num_negative_side_qp_jacobians == _num_outputs)
            _console << "Maximum log output reached, silencing output" << std::endl;
        }
        else
          _console << e.what() << std::endl;
      }
    }
  }
  diagnosticsLog("Number of element sides with negative Jacobians: " +
                     Moose::stringify(num_negative_side_qp_jacobians),
                 _check_local_jacobian,
                 num_negative_side_qp_jacobians);
}

void
MeshDiagnosticsGenerator::checkNonMatchingEdges(const std::unique_ptr<MeshBase> & mesh) const
{
  /*Algorithm Overview
    1)Prechecks
      a)This algorithm only works for 3D so check for that first
    2)Loop
      a)Loop through every element
      b)For each element get the edges associated with it
      c)For each edge check overlap with any edges of nearby elements
      d)Have check to make sure the same pair of edges are not being tested twice for overlap
    3)Overlap check
      a)Shortest line that connects both lines is perpendicular to both lines
      b)A good overview of the math for finding intersecting lines can be found
    here->paulbourke.net/geometry/pointlineplane/
  */
  if (mesh->mesh_dimension() != 3)
    mooseError("The edge intersection algorithm only works with 3D meshes");
  if (!mesh->is_serial())
    mooseError("Only serialized/replicated meshes are supported");
  unsigned int num_intersecting_edges = 0;

  // Create map of element to bounding box to avoing reinitializing the same bounding box multiple
  // times
  std::unordered_map<Elem *, BoundingBox> bounding_box_map;
  for (const auto elem : mesh->active_element_ptr_range())
  {
    const auto boundingBox = elem->loose_bounding_box();
    bounding_box_map.insert({elem, boundingBox});
  }

  std::unique_ptr<PointLocatorBase> point_locator = mesh->sub_point_locator();
  std::set<std::array<dof_id_type, 4>> overlapping_edges_nodes;
  for (const auto elem : mesh->active_element_ptr_range())
  {
    // loop through elem's nodes and find nearby elements with it
    std::set<const Elem *> candidate_elems;
    std::set<const Elem *> nearby_elems;
    for (unsigned int i = 0; i < elem->n_nodes(); i++)
    {
      (*point_locator)(elem->point(i), candidate_elems);
      nearby_elems.insert(candidate_elems.begin(), candidate_elems.end());
    }
    std::vector<std::unique_ptr<const Elem>> elem_edges(elem->n_edges());
    for (auto i : elem->edge_index_range())
      elem_edges[i] = elem->build_edge_ptr(i);
    for (const auto other_elem : nearby_elems)
    {
      // If they're the same element then there's no need to check for overlap
      if (elem->id() >= other_elem->id())
        continue;

      std::vector<std::unique_ptr<const Elem>> other_edges(other_elem->n_edges());
      for (auto j : other_elem->edge_index_range())
        other_edges[j] = other_elem->build_edge_ptr(j);
      for (auto & edge : elem_edges)
      {
        for (auto & other_edge : other_edges)
        {
          // Get nodes from edges
          const Node * n1 = edge->get_nodes()[0];
          const Node * n2 = edge->get_nodes()[1];
          const Node * n3 = other_edge->get_nodes()[0];
          const Node * n4 = other_edge->get_nodes()[1];

          // Create array<dof_id_type, 4> to check against set
          std::array<dof_id_type, 4> node_id_array = {n1->id(), n2->id(), n3->id(), n4->id()};
          std::sort(node_id_array.begin(), node_id_array.end());

          // Check if the edges have already been added to our check_edges list
          if (overlapping_edges_nodes.count(node_id_array))
          {
            continue;
          }

          // Check element/edge type
          if (edge->type() != EDGE2)
          {
            std::string element_message = "Edge of type " + Utility::enum_to_string(edge->type()) +
                                          " was found in cell " + std::to_string(elem->id()) +
                                          " which is of type " +
                                          Utility::enum_to_string(elem->type()) + '\n' +
                                          "The edge intersection check only works for EDGE2 "
                                          "elements.\nThis message will not be output again";
            mooseDoOnce(_console << element_message << std::endl);
            continue;
          }
          if (other_edge->type() != EDGE2)
            continue;

          // Now compare edge with other_edge
          Point intersection_coords;
          bool overlap = MeshBaseDiagnosticsUtils::checkFirstOrderEdgeOverlap(
              *edge, *other_edge, intersection_coords, _non_matching_edge_tol);
          if (overlap)
          {
            // Add the nodes that make up the 2 edges to the vector overlapping_edges_nodes
            overlapping_edges_nodes.insert(node_id_array);
            num_intersecting_edges += 2;
            if (num_intersecting_edges < _num_outputs)
            {
              // Print error message
              std::string elem_id = std::to_string(elem->id());
              std::string other_elem_id = std::to_string(other_elem->id());
              std::string x_coord = std::to_string(intersection_coords(0));
              std::string y_coord = std::to_string(intersection_coords(1));
              std::string z_coord = std::to_string(intersection_coords(2));
              std::string message = "Intersecting edges found between elements " + elem_id +
                                    " and " + other_elem_id + " near point (" + x_coord + ", " +
                                    y_coord + ", " + z_coord + ")";
              _console << message << std::endl;
            }
          }
        }
      }
    }
  }
  diagnosticsLog("Number of intersecting element edges: " +
                     Moose::stringify(num_intersecting_edges),
                 _check_non_matching_edges,
                 num_intersecting_edges);
}

void
MeshDiagnosticsGenerator::diagnosticsLog(std::string msg,
                                         const MooseEnum & log_level,
                                         bool problem_detected) const
{
  mooseAssert(log_level != "NO_CHECK",
              "We should not be outputting logs if the check had been disabled");
  if (log_level == "INFO" || !problem_detected)
    mooseInfoRepeated(msg);
  else if (log_level == "WARNING")
    mooseWarning(msg);
  else if (log_level == "ERROR")
    mooseError(msg);
  else
    mooseError("Should not reach here");
}
