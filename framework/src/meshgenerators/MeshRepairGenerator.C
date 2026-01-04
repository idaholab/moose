//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshRepairGenerator.h"
#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/mesh_modification.h"

registerMooseObject("MooseApp", MeshRepairGenerator);

InputParameters
MeshRepairGenerator::validParams()
{

  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription(
      "Mesh generator to perform various improvement / fixing operations on an input mesh");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Name of the mesh generator providing the mesh");

  params.addParam<bool>("fix_node_overlap", false, "Whether to merge overlapping nodes");
  params.addParam<Real>(
      "node_overlap_tol", 1e-8, "Absolute tolerance for merging overlapping nodes");

  params.addParam<bool>(
      "fix_elements_orientation", false, "Whether to flip elements with negative volumes");

  params.addParam<bool>("separate_blocks_by_element_types",
                        false,
                        "Create new blocks if multiple element types are present in a block");

  params.addParam<bool>("merge_boundary_ids_with_same_name",
                        false,
                        "Merge boundaries if they have the same name but different boundary IDs");

  params.addParam<bool>(
      "split_nonconvex_polygons", false, "Split non-convex polygons to form convex ones");
  return params;
}

MeshRepairGenerator::MeshRepairGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _fix_overlapping_nodes(getParam<bool>("fix_node_overlap")),
    _node_overlap_tol(getParam<Real>("node_overlap_tol")),
    _fix_element_orientation(getParam<bool>("fix_elements_orientation")),
    _elem_type_separation(getParam<bool>("separate_blocks_by_element_types")),
    _boundary_id_merge(getParam<bool>("merge_boundary_ids_with_same_name")),
    _split_nonconvex_polygons(getParam<bool>("split_nonconvex_polygons"))
{
  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_boundary_id_merge && !_split_nonconvex_polygons)
    mooseError("No specific item to fix. Are any of the parameters misspelled?");
}

std::unique_ptr<MeshBase>
MeshRepairGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  mesh->prepare_for_use();

  // Blanket ban on distributed. This can be relaxed for some operations if needed
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh. The mesh should not be distributed.");

  if (_fix_overlapping_nodes)
    fixOverlappingNodes(mesh);

  // Flip orientation of elements to keep positive volumes
  if (_fix_element_orientation)
    MeshTools::Modification::orient_elements(*mesh);

  // Disambiguate any block that has elements of multiple types
  if (_elem_type_separation)
    separateSubdomainsByElementType(mesh);

  // Assign a single boundary ID to boundaries that have the same name
  if (_boundary_id_merge)
    MooseMeshUtils::mergeBoundaryIDsWithSameName(*mesh);

  // Split non-convex polygons to make convex ones
  if (_split_nonconvex_polygons)
    splitNonConvexPolygons(mesh);

  mesh->set_isnt_prepared();
  return mesh;
}

void
MeshRepairGenerator::fixOverlappingNodes(std::unique_ptr<MeshBase> & mesh) const
{
  unsigned int num_fixed_nodes = 0;
  auto pl = mesh->sub_point_locator();
  pl->set_close_to_point_tol(_node_overlap_tol);

  std::unordered_set<dof_id_type> nodes_removed;
  // loop on nodes
  for (auto & node : mesh->node_ptr_range())
  {
    // this node has already been removed
    if (nodes_removed.count(node->id()))
      continue;

    // find all the elements around this node
    std::set<const Elem *> elements;
    (*pl)(*node, elements);

    for (auto & elem : elements)
    {
      bool found = false;
      for (auto & elem_node : elem->node_ref_range())
      {
        if (node->id() == elem_node.id())
        {
          found = true;
          break;
        }
      }
      if (!found)
      {
        for (auto & elem_node : elem->node_ref_range())
        {
          if (elem_node.id() == node->id())
            continue;
          const Real tol = _node_overlap_tol;
          // Compares the coordinates
          const auto x_node = (*node)(0);
          const auto x_elem_node = elem_node(0);
          const auto y_node = (*node)(1);
          const auto y_elem_node = elem_node(1);
          const auto z_node = (*node)(2);
          const auto z_elem_node = elem_node(2);

          if (MooseUtils::absoluteFuzzyEqual(x_node, x_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(y_node, y_elem_node, tol) &&
              MooseUtils::absoluteFuzzyEqual(z_node, z_elem_node, tol))
          {
            // Merging two nodes from the same element is almost never a good idea
            if (elem->get_node_index(node) != libMesh::invalid_uint)
            {
              _console << "Two overlapping nodes in element " << elem->id() << " right by "
                       << elem->vertex_average() << ".\n They will not be stitched" << std::endl;
              continue;
            }

            // Coordinates are the same but it's not the same node
            // Replace the node in the element
            const_cast<Elem *>(elem)->set_node(elem->get_node_index(&elem_node), node);
            nodes_removed.insert(elem_node.id());

            num_fixed_nodes++;
            if (num_fixed_nodes < 10)
              _console << "Stitching nodes " << *node << " and            " << elem_node
                       << std::endl;
            else if (num_fixed_nodes == 10)
              _console << "Node stitching will now proceed silently." << std::endl;
          }
        }
      }
    }
  }
  _console << "Number of overlapping nodes which got merged: " << num_fixed_nodes << std::endl;
  if (mesh->allow_renumbering())
    mesh->renumber_nodes_and_elements();
  else
  {
    mesh->remove_orphaned_nodes();
    mesh->update_parallel_id_counts();
  }
}

void
MeshRepairGenerator::separateSubdomainsByElementType(std::unique_ptr<MeshBase> & mesh) const
{
  std::set<subdomain_id_type> ids;
  mesh->subdomain_ids(ids);
  // loop on sub-domain
  for (const auto id : ids)
  {
    // Gather all the element types and blocks
    // ElemType defines an enum for geometric element types
    std::set<ElemType> types;
    // loop on elements within this sub-domain
    for (auto & elem : mesh->active_subdomain_elements_ptr_range(id))
      types.insert(elem->type());

    // This call must be performed on all processes
    auto next_block_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

    if (types.size() > 1)
    {
      subdomain_id_type i = 0;
      for (const auto type : types)
      {
        auto new_id = next_block_id + i++;
        // Create blocks when a block has multiple element types
        mesh->subdomain_name(new_id) = mesh->subdomain_name(id) + "_" + Moose::stringify(type);

        // Re-assign elements to the new blocks
        for (auto elem : mesh->active_subdomain_elements_ptr_range(id))
          if (elem->type() == type)
            elem->subdomain_id() = new_id;
      }
    }
  }
}

void
MeshRepairGenerator::splitNonConvexPolygons(std::unique_ptr<MeshBase> & mesh) const
{
  // Counters to keep track of what happened in the splitting
  unsigned int num_polygons = 0;
  unsigned int num_nonconvex = 0;
  unsigned int num_triangulated = 0;
  std::vector<Elem *> elems_to_delete;
  std::vector<std::unique_ptr<libMesh::C0Polygon>> elements_to_add_to_mesh;

  for (auto elem : mesh->element_ptr_range())
  {
    // We are not splitting non-convex quads. Maybe later.
    if (elem->type() == libMesh::C0POLYGON)
    {
      num_polygons++;

      // Lambda function to determine if an element is convex
      auto is_convex = [](const Elem * elem,
                          std::vector<std::pair<int, Real>> & obtuse_angle_nodes) -> bool
      {
        mooseAssert(elem, "Should have an elem");
        mooseAssert(obtuse_angle_nodes.empty(), "Should not be empty");

        // Find the normal to the element plane
        const auto elem_centroid = elem->true_centroid();
        const auto n_nodes = elem->n_nodes();
        Point plane_normal;
        // Two nodes could be aligned with the centroid in a non-convex polygon
        for (const auto i : make_range(n_nodes))
        {
          const auto v1 = elem_centroid - elem->point(i);
          const auto v2 = elem_centroid - elem->point((i + 1) % n_nodes);
          if (!MooseUtils::absoluteFuzzyEqual((v1.cross(v2)).norm_sq(), 0))
          {
            plane_normal = v1.cross(v2).unit();
            break;
          }
        }

        // Look for angles > 180 deg with the cross product
        bool convex = true;
        for (const auto i : make_range(n_nodes))
        {
          const auto n1 = elem->point(i);
          const auto n2 = elem->point((i + 1) % n_nodes);
          const auto n3 = elem->point((i + 2) % n_nodes);
          const auto top_dir = (n2 - n1).cross(n3 - n2);

          if (plane_normal * top_dir <= 0)
          {
            convex = false;
            if (!MooseUtils::absoluteFuzzyEqual(plane_normal * top_dir, 0))
              obtuse_angle_nodes.push_back(
                  std::make_pair<int, Real>(i, plane_normal * top_dir / top_dir.norm()));
            // n1, n2 and n3 are likely aligned
            else
              obtuse_angle_nodes.push_back(std::make_pair<int, Real>(i, -1));
          }
        }
        return convex;
      };

      std::vector<std::pair<int, Real>> parent_obtuse_angles;
      const auto convex = is_convex(elem, parent_obtuse_angles);

      if (!convex)
        num_nonconvex++;
      // Move to next one if it is convex, no need to fix it
      else
        continue;

      // Lambda function to split a polygon into two polygons at the specified nodes
      // TODO: we could try to split a polygon into more than two polygons
      auto cut_polygon = [&mesh](const Elem * elem, unsigned int node_i_cut1, unsigned node_i_cut2)
          -> std::pair<std::unique_ptr<libMesh::C0Polygon>, std::unique_ptr<libMesh::C0Polygon>>
      {
        const auto cut1 = std::min(node_i_cut1, node_i_cut2);
        const auto cut2 = std::max(node_i_cut1, node_i_cut2);

        // Count the number of sides on each side of the cut
        const auto ns1 = cut2 - cut1 + 1;
        const auto ns2 = elem->n_nodes() - ns1 + 2;
        mooseAssert(ns1 >= 3, "Should cut at least a triangle");
        mooseAssert(ns2 >= 3, "Should cut at least a triangle");

        // Create the children and add their nodes
        auto child1 = std::make_unique<libMesh::C0Polygon>(ns1);
        for (const auto i_n : make_range(cut1, cut2 + 1))
          child1->set_node(i_n - cut1, mesh->node_ptr(elem->node_ptr(i_n)->id()));

        auto child2 = std::make_unique<libMesh::C0Polygon>(ns2);
        for (const auto i_n : make_range(cut2, elem->n_nodes() + cut1 + 1))
          child2->set_node((i_n - cut2) % child2->n_nodes(),
                           mesh->node_ptr(elem->node_ptr(i_n % elem->n_nodes())->id()));

        return std::make_pair(std::move(child1), std::move(child2));
      };

      // If not convex, try to split it.
      // Let's first try to split it recursively at every large angle
      // NOTE: These vectors of unique pointer keep the memory ownership of the elements
      // during the loop attempting to fix the polygon with the heuristic below
      bool failed = false;
      std::vector<std::unique_ptr<libMesh::C0Polygon>> elements_to_add_to_mesh_temp;
      std::vector<std::pair<std::unique_ptr<Elem>, std::vector<std::pair<int, Real>>>> elems_to_cut;

      // Create a clone of the first element to simplify logic
      std::unique_ptr<libMesh::C0Polygon> base_elem =
          std::make_unique<libMesh::C0Polygon>(elem->n_nodes());
      for (const auto i_n : make_range(elem->n_nodes()))
        base_elem->set_node(i_n, elem->node_ptr(i_n));
      elems_to_cut.push_back(std::make_pair(std::move(base_elem), parent_obtuse_angles));

      while (!failed && !elems_to_cut.empty())
      {
        auto & [current_elem, large_angles] = elems_to_cut.back();

        // Split the element on one side at the node with the worst obtuse angle
        // TODO: try all of them instead to see if a single cut can fix the element
        auto worst_angle_it =
            std::min_element(large_angles.begin(),
                             large_angles.end(),
                             [](std::pair<int, Real> lhs, std::pair<int, Real> rhs) -> bool
                             { return lhs.second < rhs.second; });
        unsigned int worst_angle_i = (*worst_angle_it).first;
        const auto n_nodes = current_elem->n_nodes();

        // Split the element on the other side at roughly the opposite node
        // TODO: we could try all of the 'other' nodes here too to see if a single cut can fix the
        // element NOTE: if trying multiple cuts (not implemented), we should remove the opposite
        // from the large_angles
        const auto opposite = ((worst_angle_i + n_nodes / 2) % n_nodes);

        auto [child1, child2] = cut_polygon(current_elem.get(), worst_angle_i, opposite);

        // Check the two fragments
        std::vector<std::pair<int, Real>> child1_large_angles;
        bool is_convex1 = is_convex(child1.get(), child1_large_angles);
        std::vector<std::pair<int, Real>> child2_large_angles;
        bool is_convex2 = is_convex(child2.get(), child2_large_angles);

        // Can we keep going?
        if (child1->n_nodes() < 4 && !is_convex1)
          failed = true;
        if (child2->n_nodes() < 4 && !is_convex2)
          failed = true;

        // We managed to cut the elements into a convex part, but it's flipped
        // so the element is likely "outside" of the starting element
        if (is_convex1 && child1->volume() <= 0)
          failed = true;
        if (is_convex2 && child2->volume() <= 0)
          failed = true;

        // Current element is done
        // NOTE: if trying multiple cuts (not implemented), we should not erase yet, we should
        // instead only remove the cut we tried from the 'large_angles' vector used to pick cuts
        large_angles.erase(worst_angle_it);
        elems_to_cut.pop_back();

        // Add non convex elements to the list of elements to cut
        if (!is_convex1)
          elems_to_cut.push_back(std::make_pair(std::move(child1), child1_large_angles));
        if (!is_convex2)
          elems_to_cut.push_back(std::make_pair(std::move(child2), child2_large_angles));

        // Add convex elements to the mesh
        // NOTE: if trying multiple cuts, we should not do this yet
        if (is_convex1)
          elements_to_add_to_mesh_temp.push_back(std::move(child1));
        if (is_convex2)
          elements_to_add_to_mesh_temp.push_back(std::move(child2));
      }
      bool fixed_it = !failed;

      // Keep all the cut elements, to be added at the end to avoid invalidating iterators
      if (fixed_it)
        for (auto & elem_uptr : elements_to_add_to_mesh_temp)
          elements_to_add_to_mesh.push_back(std::move(elem_uptr));

      // Heuristic did not work, just use a triangulation
      // NOTE: This is not the triangulation of the polygon. It is simple though
      if (!fixed_it)
      {
        const auto centroid_node = mesh->add_point(elem->true_centroid());
        num_triangulated++;
        const auto * poly = dynamic_cast<libMesh::C0Polygon *>(elem);
        const auto n_sides = poly->n_sides();
        for (const auto i_tr : make_range(n_sides))
        {
          auto new_elem = std::make_unique<libMesh::C0Polygon>(3);
          new_elem->set_node(0, mesh->node_ptr(elem->node_ptr(i_tr)->id()));
          new_elem->set_node(1, centroid_node);
          new_elem->set_node(2, mesh->node_ptr(elem->node_ptr((i_tr + 1) % n_sides)->id()));

          // Check for degenerate case
          if (MooseUtils::absoluteFuzzyEqual(
                  (*(Point *)centroid_node - *(Point *)elem->node_ptr(i_tr))
                      .cross(*(Point *)centroid_node -
                             *(Point *)elem->node_ptr((i_tr + 1) % n_sides))
                      .norm_sq(),
                  0))
            mooseError("Manual triangulation failed as two consecutive nodes are aligned with the "
                       "centroid");

          elements_to_add_to_mesh.push_back(std::move(new_elem));
        }
      }
      // Element got fixed, can be deleted after (can't while using the range)
      elems_to_delete.push_back(elem);
    }
  }
  // Delete the original element
  for (auto elem : elems_to_delete)
    mesh->delete_elem(elem);
  // Add the new ones
  for (auto & new_elem_ptr : elements_to_add_to_mesh)
    mesh->add_elem(std::move(new_elem_ptr));

  _console << "Number of non-convex polygons which got split into convex polygons: "
           << num_nonconvex << std::endl;
  if (num_triangulated)
    _console << "Number of non-convex polygons split using a triangulation: " << num_triangulated
             << ", using heuristic: " << num_nonconvex - num_triangulated << std::endl;
  if (!num_polygons)
    mooseWarning("No C0 polygons in mesh: the polyon convexity fix did nothing");
}
