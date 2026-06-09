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
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundaries", "The sideset(s) to coarsen the mesh along");
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
    _boundaries(getParam<std::vector<BoundaryName>>("boundaries")),
    _verbose(getParam<bool>("verbose"))
{
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

  // Resolve the sideset names to ids
  const auto boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, _boundaries, false);
  const std::set<boundary_id_type> boundary_id_set(boundary_ids.begin(), boundary_ids.end());

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

  if (boundary_node_neighbors.empty())
    paramError("boundaries", "No sides were found for the requested sideset(s)");

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

      // Validate the collapse and classify each incident element as kept (re-pointed) or
      // degenerate (to be deleted). A degenerate element must be a TRI3; a re-pointed element
      // must not become degenerate or flip its normal.
      bool valid = true;
      std::vector<Elem *> degenerate;
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
        bool degenerates = false;
        std::vector<Point> orig_pts, new_pts;
        for (const auto & node : elem->node_ref_range())
        {
          orig_pts.push_back(node);
          if (node.id() == node_id)
            new_pts.push_back(a_point);
          else
          {
            if (node.id() == target_id)
              degenerates = true;
            new_pts.push_back(node);
          }
        }

        if (degenerates)
        {
          // We only delete triangles. A degenerating quad would require a quad->tri conversion
          // (and boundary bookkeeping of its other sides), so we decline the collapse instead.
          if (elem->type() != TRI3)
          {
            valid = false;
            break;
          }
          degenerate.push_back(elem);
        }
        else
        {
          // Re-pointed element: reject the collapse if it would invert or nearly flatten it
          const Point orig_n = newellNormal(orig_pts);
          const Point new_n = newellNormal(new_pts);
          if (new_n * orig_n <= 1e-6 * orig_n.norm_sq())
          {
            valid = false;
            break;
          }
        }
      }

      // A clean collapse removes exactly one triangle per side of the sideset
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

  return dynamic_pointer_cast<MeshBase>(mesh);
}
