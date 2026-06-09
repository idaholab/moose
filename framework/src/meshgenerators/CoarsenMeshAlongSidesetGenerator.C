//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarsenMeshAlongSidesetGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", CoarsenMeshAlongSidesetGenerator);

InputParameters
CoarsenMeshAlongSidesetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription(
      "Coarsens a 2D-element (TRI3/QUAD4) surface mesh along a sideset by collapsing alternate "
      "boundary nodes. The sideset may be internal: elements on both sides of it are coarsened "
      "and the sideset itself is preserved. Apply the generator multiple times for additional "
      "coarsening.");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to coarsen");
  params.addParam<std::vector<BoundaryName>>("boundaries",
                                             "The sideset(s) to coarsen the mesh along");
  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundaries",
      "Coarsen the mesh along all of its sidesets except these. Mutually exclusive with "
      "'boundaries'");
  params.addRangeCheckedParam<Real>(
      "max_normal_deviation",
      "max_normal_deviation >= 0 & max_normal_deviation <= 180",
      "Maximum angle, in degrees, between the normals of the two elements merged together. "
      "Merges exceeding it are skipped, which preserves features/corners");
  params.addRangeCheckedParam<Real>(
      "max_merged_side_length",
      "max_merged_side_length > 0",
      "Maximum length of the side created along the sideset by merging two elements. "
      "Merges exceeding it are skipped");
  params.addRangeCheckedParam<Real>(
      "max_merged_element_area",
      "max_merged_element_area > 0",
      "Maximum area of an element created by merging two elements. Merges exceeding it are "
      "skipped");
  params.addParam<bool>(
      "coarsen_more_than_two_elements",
      false,
      "Whether to coarsen iteratively in a single invocation so that more than two elements can "
      "be merged together. The amount of coarsening is then bounded by the merge criteria");
  params.addParam<bool>(
      "verbose",
      false,
      "Whether to make the mesh generator output details of its actions on the console");
  return params;
}

CoarsenMeshAlongSidesetGenerator::CoarsenMeshAlongSidesetGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundaries(isParamValid("boundaries") ? getParam<std::vector<BoundaryName>>("boundaries")
                                           : std::vector<BoundaryName>{}),
    _exclude_boundaries(isParamValid("exclude_boundaries")
                            ? getParam<std::vector<BoundaryName>>("exclude_boundaries")
                            : std::vector<BoundaryName>{}),
    _has_max_normal_deviation(isParamValid("max_normal_deviation")),
    _max_normal_deviation(_has_max_normal_deviation ? getParam<Real>("max_normal_deviation") : 0),
    _has_max_side_length(isParamValid("max_merged_side_length")),
    _max_merged_side_length(_has_max_side_length ? getParam<Real>("max_merged_side_length") : 0),
    _has_max_element_area(isParamValid("max_merged_element_area")),
    _max_merged_element_area(_has_max_element_area ? getParam<Real>("max_merged_element_area") : 0),
    _coarsen_more_than_two_elements(getParam<bool>("coarsen_more_than_two_elements")),
    _verbose(getParam<bool>("verbose"))
{
  if (_boundaries.empty() == _exclude_boundaries.empty())
    paramError("boundaries",
               "Exactly one of 'boundaries' and 'exclude_boundaries' must be provided");
}

namespace
{
/// Area-weighted (Newell) normal of the polygon described by 'pts'. Its magnitude is
/// proportional to the polygon area and its direction encodes the orientation, so it
/// serves to detect both element degeneracy (near-zero magnitude) and inversion (flip).
Point
newellNormal(const std::vector<Point> & pts)
{
  Point n(0, 0, 0);
  for (const auto i : index_range(pts))
  {
    const Point & current = pts[i];
    const Point & next = pts[(i + 1) % pts.size()];
    n(0) += (current(1) - next(1)) * (current(2) + next(2));
    n(1) += (current(2) - next(2)) * (current(0) + next(0));
    n(2) += (current(0) - next(0)) * (current(1) + next(1));
  }
  return n;
}
}

std::unique_ptr<MeshBase>
CoarsenMeshAlongSidesetGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (!mesh->is_serial())
    paramError("input", "Input mesh must not be distributed");
  if (mesh->mesh_dimension() != 2)
    paramError("input",
               "Only meshes of 2D elements (TRI3/QUAD4) are supported, but the input mesh "
               "dimension is " +
                   std::to_string(mesh->mesh_dimension()));

  if (!mesh->is_prepared())
    mesh->prepare_for_use();

  const auto & boundary_info = mesh->get_boundary_info();

  // Resolve the sideset names to the set of ids to coarsen along, either directly or by excluding
  // the requested sidesets from all the mesh sidesets
  std::set<boundary_id_type> boundary_id_set;
  if (!_boundaries.empty())
  {
    const auto boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _boundaries, false);
    boundary_id_set.insert(boundary_ids.begin(), boundary_ids.end());
  }
  else
  {
    boundary_id_set = boundary_info.get_side_boundary_ids();
    const auto exclude_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _exclude_boundaries, false);
    for (const auto id : exclude_ids)
      boundary_id_set.erase(id);
  }

  // Verify at least one side exists for the requested sidesets
  bool found_side = false;
  for (const auto & t : boundary_info.build_side_list())
    if (boundary_id_set.count(std::get<2>(t)))
    {
      found_side = true;
      break;
    }
  if (!found_side)
    paramError("boundaries", "No sides were found for the requested sideset(s)");

  // Run the coarsening pass once, or repeatedly until no collapse remains, so that more than two
  // elements may be merged together
  unsigned int collapsed = 0;
  do
    collapsed = coarsenAlongSidesets(mesh, boundary_id_set);
  while (_coarsen_more_than_two_elements && collapsed > 0);

  return dynamic_pointer_cast<MeshBase>(mesh);
}

unsigned int
CoarsenMeshAlongSidesetGenerator::coarsenAlongSidesets(
    std::unique_ptr<MeshBase> & mesh, const std::set<boundary_id_type> & boundary_id_set)
{
  const auto & boundary_info = mesh->get_boundary_info();

  // Gather the unique boundary edges (an internal sideset lists each edge twice, once per side)
  // and from them the boundary-node adjacency along the sideset curve(s)
  std::map<dof_id_type, std::set<dof_id_type>> boundary_node_neighbors;
  for (const auto & [elem_id, side, bid] : boundary_info.build_side_list())
  {
    if (!boundary_id_set.count(bid))
      continue;
    const Elem * elem = mesh->elem_ptr(elem_id);
    const auto edge = elem->build_side_ptr(side);
    const auto n0 = edge->node_id(0);
    const auto n1 = edge->node_id(1);
    boundary_node_neighbors[n0].insert(n1);
    boundary_node_neighbors[n1].insert(n0);
  }

  // Map every node to the ids of the elements referencing it. We use ids (not pointers) so that
  // entries pointing at elements deleted earlier in the pass can be safely skipped.
  std::map<dof_id_type, std::vector<dof_id_type>> node_to_elems;
  for (const auto & elem : mesh->active_element_ptr_range())
    for (const auto & node : elem->node_ref_range())
      node_to_elems[node.id()].push_back(elem->id());

  // Greedy independent set: collapse a boundary node onto a neighbor, lock that node and both of
  // its boundary neighbors so the kept collapses do not interact (this yields the alternating,
  // ~2x coarsening pattern).
  std::set<dof_id_type> locked;
  std::vector<Elem *> elems_to_delete;
  unsigned int num_collapsed = 0;

  for (const auto & [node_id, neighbors] : boundary_node_neighbors)
  {
    // Only collapse nodes interior to a sideset polyline (skip endpoints and junctions)
    if (neighbors.size() != 2 || locked.count(node_id))
      continue;

    const Node * b_node = mesh->node_ptr(node_id);

    // Try to collapse onto each boundary neighbor, preferring the unlocked one with the lower id
    for (const auto target_id : neighbors)
    {
      if (locked.count(target_id))
        continue;

      const Point a_point = *mesh->node_ptr(target_id);

      // The collapse merges the two sideset edges (target,node) and (node,other) into the single
      // edge (target,other), regardless of which way we collapse
      dof_id_type other_id = DofObject::invalid_id;
      for (const auto n : neighbors)
        if (n != target_id)
          other_id = n;

      // Criterion: maximum length of the side created along the sideset
      if (_has_max_side_length &&
          (a_point - *mesh->node_ptr(other_id)).norm() > _max_merged_side_length)
        continue;

      // Validate the collapse and classify each incident element as kept (re-pointed) or
      // degenerate (to be deleted). A degenerate element must be a TRI3; a re-pointed element
      // must not become degenerate or flip its normal.
      struct ElemInfo
      {
        Elem * elem;
        bool degenerate;
        std::vector<dof_id_type> ids;
        std::vector<Point> orig_pts;
        std::vector<Point> new_pts;
      };
      bool valid = true;
      std::vector<ElemInfo> infos;
      for (const auto incident_id : node_to_elems[node_id])
      {
        Elem * elem = mesh->elem_ptr(incident_id);
        if (!elem) // deleted earlier in this pass
          continue;
        if (elem->type() != TRI3 && elem->type() != QUAD4)
        {
          valid = false;
          break;
        }

        // Does the element already contain the target node? If so the collapse degenerates it.
        ElemInfo info{elem, false, {}, {}, {}};
        for (const auto & node : elem->node_ref_range())
        {
          info.ids.push_back(node.id());
          info.orig_pts.push_back(node);
          if (node.id() == node_id)
            info.new_pts.push_back(a_point);
          else
          {
            if (node.id() == target_id)
              info.degenerate = true;
            info.new_pts.push_back(node);
          }
        }

        if (info.degenerate)
        {
          // We only delete triangles. A degenerating quad would require a quad->tri conversion
          // (and boundary bookkeeping of its other sides), so we decline the collapse instead.
          if (elem->type() != TRI3)
          {
            valid = false;
            break;
          }
        }
        else
        {
          // Re-pointed element: reject the collapse if it would invert or nearly flatten it
          const Point orig_n = newellNormal(info.orig_pts);
          const Point new_n = newellNormal(info.new_pts);
          if (new_n * orig_n <= 1e-6 * orig_n.norm_sq())
          {
            valid = false;
            break;
          }
          // Criterion: maximum area of the merged element
          if (_has_max_element_area && 0.5 * new_n.norm() > _max_merged_element_area)
          {
            valid = false;
            break;
          }
        }
        infos.push_back(info);
      }

      // Criterion: maximum normal deviation between the two elements being merged. Each deleted
      // triangle (target,node,apex) merges with the re-pointed element sharing its apex node.
      if (valid && _has_max_normal_deviation)
      {
        const Real cos_threshold = std::cos(_max_normal_deviation * libMesh::pi / 180.0);
        for (const auto & d : infos)
        {
          if (!d.degenerate)
            continue;
          dof_id_type apex_id = DofObject::invalid_id;
          for (const auto id : d.ids)
            if (id != target_id && id != node_id)
              apex_id = id;
          for (const auto & r : infos)
          {
            if (r.degenerate || std::find(r.ids.begin(), r.ids.end(), apex_id) == r.ids.end())
              continue;
            const Point nd = newellNormal(d.orig_pts);
            const Point nr = newellNormal(r.orig_pts);
            const Real denom = nd.norm() * nr.norm();
            if (denom > 0 && (nd * nr) / denom < cos_threshold)
              valid = false;
          }
        }
      }

      // A clean collapse removes exactly one triangle per side of the sideset
      std::vector<Elem *> degenerate;
      for (const auto & info : infos)
        if (info.degenerate)
          degenerate.push_back(info.elem);
      if (!valid || degenerate.empty())
        continue;

      // Commit: re-point every incident element from the collapsed node to the target node, and
      // mark the degenerate triangles for deletion.
      const std::set<Elem *> degenerate_set(degenerate.begin(), degenerate.end());
      for (const auto incident_id : node_to_elems[node_id])
      {
        Elem * elem = mesh->elem_ptr(incident_id);
        if (!elem || degenerate_set.count(elem))
          continue;
        elem->set_node(elem->get_node_index(b_node)) = mesh->node_ptr(target_id);
      }
      for (auto elem : degenerate)
        elems_to_delete.push_back(elem);

      locked.insert(node_id);
      for (const auto neighbor : neighbors)
        locked.insert(neighbor);
      num_collapsed++;
      break;
    }
  }

  for (auto elem : elems_to_delete)
    mesh->delete_elem(elem);

  if (_verbose)
    _console << name() << ": collapsed " << num_collapsed << " boundary node(s), deleted "
             << elems_to_delete.size() << " element(s)." << std::endl;

  // Orphaned nodes (the collapsed ones) are removed while preparing the mesh for use
  mesh->contract();
  mesh->prepare_for_use();

  return num_collapsed;
}
