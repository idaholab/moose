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
#include "libmesh/boundary_info.h"
#include "libmesh/face_c0polygon.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_quad4.h"

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
      "fix_sliver_triangles",
      false,
      "Whether to repair sliver (near-degenerate) first-order 2D elements (TRI3, QUAD4, polygons). "
      "Each sliver is removed and its far-side vertices are inserted into the longest-edge "
      "neighbor, so the surface stays conformal. The neighbor is promoted accordingly: a triangle "
      "becomes a quadrilateral, and anything else becomes a polygon.");
  params.addRangeCheckedParam<Real>(
      "sliver_triangle_area_fraction",
      1e-10,
      "sliver_triangle_area_fraction>=0",
      "A 2D element whose area is below this fraction of the mesh surface-area scale is treated as "
      "a sliver (set to 0 to disable this test). Only used when 'fix_sliver_triangles' is set.");
  params.addRangeCheckedParam<Real>(
      "sliver_triangle_flap_tol",
      0.02,
      "sliver_triangle_flap_tol>=0",
      "A 2D element is treated as a sliver if every vertex other than the two ends of its longest "
      "edge lies within this fraction of the longest-edge length from that edge, projecting onto "
      "its interior (set to 0 to disable this test). Only used when 'fix_sliver_triangles' is "
      "set.");

  params.addParam<bool>(
      "renumber_contiguously",
      false,
      "Whether to renumber the elements of the mesh so the numbering is contiguous");

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
    _split_nonconvex_polygons(getParam<bool>("split_nonconvex_polygons")),
    _fix_sliver_triangles(getParam<bool>("fix_sliver_triangles")),
    _sliver_area_tol(getParam<Real>("sliver_triangle_area_fraction")),
    _sliver_flap_tol(getParam<Real>("sliver_triangle_flap_tol"))
{
  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_boundary_id_merge && !getParam<bool>("renumber_contiguously") && !_split_nonconvex_polygons)

  if (!_fix_overlapping_nodes && !_fix_element_orientation && !_elem_type_separation &&
      !_boundary_id_merge && !_fix_sliver_triangles && !getParam<bool>("renumber_contiguously"))
    mooseError("No specific item to fix. Are any of the parameters misspelled?");
}

std::unique_ptr<MeshBase>
MeshRepairGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // We're trying to repair a potentially broken mesh; we'll just
  // start with a full prepare rather than trying to be efficient and
  // risking missing something.
  mesh->prepare_for_use();

  // Blanket ban on distributed. This can be relaxed for some operations if needed
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh. The mesh should not be distributed.");

  if (_fix_overlapping_nodes)
    fixOverlappingNodes(mesh);

  // Repair sliver elements by absorbing them into their longest-edge neighbor
  if (_fix_sliver_triangles)
    repairSlivers(mesh);

  // Flip orientation of elements to keep positive volumes
  if (_fix_element_orientation)
    MeshTools::Modification::orient_elements(*mesh);

  // Disambiguate any block that has elements of multiple types
  if (_elem_type_separation)
    separateSubdomainsByElementType(mesh);

  // Assign a single boundary ID to boundaries that have the same name
  if (_boundary_id_merge)
    MooseMeshUtils::mergeBoundaryIDsWithSameName(*mesh);

  // Renumber the mesh despite any mesh flag
  if (getParam<bool>("renumber_contiguously"))
  {
    const auto prev_status = mesh->allow_renumbering();
    mesh->allow_renumbering(true);
    mesh->renumber_nodes_and_elements();
    mesh->allow_renumbering(prev_status);
  }

  // Split non-convex polygons to make convex ones
  if (_split_nonconvex_polygons)
    splitNonConvexPolygons(mesh);

  mesh->unset_is_prepared();
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
<<<<<<< HEAD
MeshRepairGenerator::splitNonConvexPolygons(std::unique_ptr<MeshBase> & mesh) const
{
  // Counters to keep track of what happened in the splitting
  unsigned int num_polygons = 0;
  unsigned int num_nonconvex = 0;
  unsigned int num_triangulated = 0;
  std::vector<Elem *> elems_to_delete;
  std::vector<std::unique_ptr<libMesh::C0Polygon>> elements_to_add_to_mesh;

  // Missing parallel packing for polys
  if (!mesh->is_serial())
    mooseError("MeshRepairGenerator requires a serial mesh for this operation. "
               "The mesh should not be distributed.");

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
        const auto elem_centroid = elem->vertex_average();
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
        {
          elem_uptr->inherit_data_from(*elem);
          elements_to_add_to_mesh.push_back(std::move(elem_uptr));
        }

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

          new_elem->inherit_data_from(*elem);
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

void
MeshRepairGenerator::repairSliverTriangles(std::unique_ptr<MeshBase> & mesh) const
=======
MeshRepairGenerator::repairSlivers(std::unique_ptr<MeshBase> & mesh) const
>>>>>>> 9e1158556d9 (Generalize MeshRepairGenerator sliver repair to all 2D element types)
{
  // Surface-area scale of the whole mesh, used by the area-based sliver test
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  const Point ext = bbox.max() - bbox.min();
  const Real surface_scale =
      std::abs(ext(0) * ext(1)) + std::abs(ext(0) * ext(2)) + std::abs(ext(1) * ext(2));
  const Real area_thresh = std::max(surface_scale, Real(1e-30)) * _sliver_area_tol;

  // Index of the longest edge (between vertices lng and lng+1) of a 2D element
  auto longestEdge = [](const Elem & e)
  {
    const auto nv = e.n_vertices();
    unsigned int lng = 0;
    Real longest = -1;
    for (const auto i : make_range(nv))
    {
      const Real l = (e.point((i + 1) % nv) - e.point(i)).norm();
      if (l > longest)
      {
        longest = l;
        lng = i;
      }
    }
    return lng;
  };
  // Area of a (planar) 2D element, by fanning from its first vertex
  auto elemArea = [](const Elem & e)
  {
    const auto nv = e.n_vertices();
    Point area_vec;
    for (const auto i : make_range(1u, nv - 1))
      area_vec += (e.point(i) - e.point(0)).cross(e.point(i + 1) - e.point(0));
    return 0.5 * area_vec.norm();
  };
  // Whether vertex P lies within tol*|A-B| of segment A-B (distance to the clamped segment, so a
  // vertex sitting above an endpoint of a thin strip still counts)
  auto nearEdge = [](const Point & P, const Point & A, const Point & B, const Real tol)
  {
    const Point AB = B - A;
    const Real elen_sq = AB.norm_sq();
    if (elen_sq <= 0)
      return false;
    const Real t = std::max(Real(0), std::min(Real(1), ((P - A) * AB) / elen_sq));
    return (P - (A + t * AB)).norm() < tol * std::sqrt(elen_sq);
  };
  // A 2D element is a sliver if, against its longest edge, every other vertex is nearly on that
  // edge (flap test) or the element area is negligible (area test). Either test can be disabled by
  // setting its tolerance to 0.
  auto isSliver = [&](const Elem & e, const unsigned int lng)
  {
    const auto nv = e.n_vertices();
    const Point & A = e.point(lng);
    const Point & B = e.point((lng + 1) % nv);
    if ((B - A).norm() == 0)
      return false;
    if (_sliver_area_tol > 0 && elemArea(e) < area_thresh)
      return true;
    if (_sliver_flap_tol > 0)
    {
      for (const auto k : make_range(nv))
        if (k != lng && k != (lng + 1) % nv && !nearEdge(e.point(k), A, B, _sliver_flap_tol))
          return false;
      return true;
    }
    return false;
  };
  // First-order linear 2D element (TRI3, QUAD4, C0POLYGON): every node is a vertex
  auto isLinear2D = [](const Elem & e) { return e.dim() == 2 && e.n_nodes() == e.n_vertices(); };

  // Undirected edge (sorted node-id pair) -> ids of the 2D elements using it, plus the list of
  // sliver elements present in the original mesh
  auto edge_key = [](dof_id_type a, dof_id_type b)
  { return std::make_pair(std::min(a, b), std::max(a, b)); };
  std::map<std::pair<dof_id_type, dof_id_type>, std::vector<dof_id_type>> edge_to_elems;
  std::vector<dof_id_type> sliver_ids;
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    if (!isLinear2D(*elem))
      continue;
    const auto nv = elem->n_vertices();
    for (const auto i : make_range(nv))
      edge_to_elems[edge_key(elem->node_id(i), elem->node_id((i + 1) % nv))].push_back(elem->id());
    if (isSliver(*elem, longestEdge(*elem)))
      sliver_ids.push_back(elem->id());
  }

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  std::unordered_set<dof_id_type> touched;
  std::size_t num_repaired = 0;

  // Repair each sliver present in the original mesh once. Elements created by a repair are never
  // re-examined, so this terminates and the surface stays conformal: removing the sliver and
  // inserting its far-side vertex chain into the neighbor moves the shared edges onto the neighbor.
  for (const auto sid : sliver_ids)
  {
    if (touched.count(sid))
      continue;
    Elem * s = mesh->elem_ptr(sid);
    if (!s)
      continue;
    const auto nv = s->n_vertices();
    const unsigned int lng = longestEdge(*s);
    if (!isSliver(*s, lng))
      continue;

    Node * A = s->node_ptr(lng);
    Node * B = s->node_ptr((lng + 1) % nv);
    // The sliver's other vertices, in order from just after B around to just before A. They get
    // inserted into the neighbor's shared edge (going B -> chain -> A).
    std::vector<Node *> chain;
    for (const auto off : make_range(2u, nv))
      chain.push_back(s->node_ptr((lng + off) % nv));

    // Find a 2D neighbor sharing the longest edge A-B
    Elem * neighbor = nullptr;
    for (const auto nid : libmesh_map_find(edge_to_elems, edge_key(A->id(), B->id())))
    {
      if (nid == sid || touched.count(nid))
        continue;
      Elem * cand = mesh->elem_ptr(nid);
      if (cand && isLinear2D(*cand))
      {
        neighbor = cand;
        break;
      }
    }
    // The longest edge is on a surface boundary (or its neighbor was already repaired): we cannot
    // absorb the sliver into a neighbor, so leave it in place
    if (!neighbor)
      continue;

    // Local side of the neighbor that is the shared edge A-B
    const auto mv = neighbor->n_vertices();
    unsigned int eidx = libMesh::invalid_uint;
    for (const auto e : make_range(mv))
    {
      const auto a = neighbor->node_id(e), b = neighbor->node_id((e + 1) % mv);
      if ((a == A->id() && b == B->id()) || (a == B->id() && b == A->id()))
      {
        eidx = e;
        break;
      }
    }
    if (eidx == libMesh::invalid_uint)
      continue; // should not happen
    const subdomain_id_type sub = neighbor->subdomain_id();
    const dof_id_type neighbor_id = neighbor->id();

    // Build the neighbor's new vertex list, inserting the chain into its shared edge. If the
    // neighbor traverses the edge B->A, insert the chain as-is; if A->B, insert it reversed.
    const bool edge_is_BA = (neighbor->node_id(eidx) == B->id());
    std::vector<Node *> new_nodes;
    new_nodes.reserve(mv + chain.size());
    for (const auto e : make_range(mv))
    {
      new_nodes.push_back(neighbor->node_ptr(e));
      if (e == eidx)
      {
        if (edge_is_BA)
          new_nodes.insert(new_nodes.end(), chain.begin(), chain.end());
        else
          new_nodes.insert(new_nodes.end(), chain.rbegin(), chain.rend());
      }
    }

    // Capture the boundary ids on the sides of the sliver and the neighbor, keyed by node-id pair,
    // so we can put them back on the surviving edges
    std::map<std::pair<dof_id_type, dof_id_type>, std::vector<boundary_id_type>> edge_bcs;
    std::vector<boundary_id_type> side_ids;
    for (const Elem * t : {const_cast<const Elem *>(s), const_cast<const Elem *>(neighbor)})
      for (const auto e : make_range(t->n_vertices()))
      {
        boundary_info.boundary_ids(t, cast_int<unsigned short>(e), side_ids);
        if (!side_ids.empty())
        {
          auto & dst = edge_bcs[edge_key(t->node_id(e), t->node_id((e + 1) % t->n_vertices()))];
          dst.insert(dst.end(), side_ids.begin(), side_ids.end());
        }
      }
    boundary_info.remove(s);
    boundary_info.remove(neighbor);

    // Create the promoted neighbor: a quad if it now has 4 vertices, otherwise a polygon
    std::unique_ptr<Elem> promoted;
    if (new_nodes.size() == 4)
      promoted = std::make_unique<Quad4>();
    else
      promoted = std::make_unique<libMesh::C0Polygon>(new_nodes.size());
    for (const auto i : index_range(new_nodes))
      promoted->set_node(i, new_nodes[i]);
    promoted->subdomain_id() = sub;
    Elem * added = mesh->add_elem(std::move(promoted));

    // Restore the captured boundary ids onto the matching edges of the new element
    for (const auto e : make_range(added->n_vertices()))
    {
      auto it =
          edge_bcs.find(edge_key(added->node_id(e), added->node_id((e + 1) % added->n_vertices())));
      if (it != edge_bcs.end())
        for (const auto bid : it->second)
          boundary_info.add_side(added, cast_int<unsigned short>(e), bid);
    }

    mesh->delete_elem(s);
    mesh->delete_elem(neighbor);
    touched.insert(sid);
    touched.insert(neighbor_id);
    ++num_repaired;
  }

  if (num_repaired)
  {
    mesh->prepare_for_use();
    _console << "Number of sliver elements repaired: " << num_repaired << std::endl;
  }
}
