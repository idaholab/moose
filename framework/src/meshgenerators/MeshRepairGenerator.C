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
#include "libmesh/face_tri3.h"
#include "libmesh/face_c0polygon.h"

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
      "Whether to repair sliver (near-degenerate) TRI3 elements. Each sliver is removed and the "
      "neighbor triangle sharing its longest edge is split at the sliver's opposite vertex, so the "
      "surface stays conformal.");
  params.addRangeCheckedParam<Real>(
      "sliver_triangle_area_fraction",
      1e-10,
      "sliver_triangle_area_fraction>=0",
      "A TRI3 whose area is below this fraction of the mesh surface-area scale is treated as a "
      "sliver (set to 0 to disable this test). Only used when 'fix_sliver_triangles' is set.");
  params.addRangeCheckedParam<Real>(
      "sliver_triangle_flap_tol",
      0.02,
      "sliver_triangle_flap_tol>=0",
      "A TRI3 is treated as a sliver if the vertex opposite its longest edge lies within this "
      "fraction of the longest-edge length from that edge, projecting onto its interior (set to 0 "
      "to disable this test). Only used when 'fix_sliver_triangles' is set.");

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

  // Repair sliver triangles by splitting their longest-edge neighbor
  if (_fix_sliver_triangles)
    repairSliverTriangles(mesh);

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
{
  // Surface-area scale of the whole mesh, used by the area-based sliver test
  const auto bbox = MeshTools::create_bounding_box(*mesh);
  const Point ext = bbox.max() - bbox.min();
  const Real surface_scale =
      std::abs(ext(0) * ext(1)) + std::abs(ext(0) * ext(2)) + std::abs(ext(1) * ext(2));
  const Real area_thresh = std::max(surface_scale, Real(1e-30)) * _sliver_area_tol;

  // Determine the longest edge (between local nodes lng and lng+1) of a TRI3 and whether it is a
  // sliver per either of the two tests
  auto analyze = [&](const Elem & tri, unsigned int & lng, bool & is_sliver)
  {
    const Real le[3] = {(tri.point(1) - tri.point(0)).norm(),
                        (tri.point(2) - tri.point(1)).norm(),
                        (tri.point(0) - tri.point(2)).norm()};
    lng = 0;
    if (le[1] > le[lng])
      lng = 1;
    if (le[2] > le[lng])
      lng = 2;
    const Real elen = le[lng];
    is_sliver = false;
    if (elen <= 0)
      return;
    // A is the start of the longest edge, B its end, C the opposite (apex) vertex
    const Point & A = tri.point(lng);
    const Point & B = tri.point((lng + 1) % 3);
    const Point & C = tri.point((lng + 2) % 3);
    // Area-based test
    if (_sliver_area_tol > 0 && 0.5 * (B - A).cross(C - A).norm() < area_thresh)
    {
      is_sliver = true;
      return;
    }
    // Geometric-flap test: apex near, and projecting onto the interior of, the longest edge
    if (_sliver_flap_tol > 0)
    {
      const Point AB = B - A;
      const Real t = ((C - A) * AB) / (elen * elen);
      const Point proj = A + std::max(Real(0), std::min(Real(1), t)) * AB;
      if (t > 0.001 && t < 0.999 && (C - proj).norm() < _sliver_flap_tol * elen)
        is_sliver = true;
    }
  };

  // Undirected edge (sorted node-id pair) -> ids of the TRI3 elements using it, plus the list of
  // sliver elements present in the original mesh
  auto edge_key = [](dof_id_type a, dof_id_type b)
  { return std::make_pair(std::min(a, b), std::max(a, b)); };
  std::map<std::pair<dof_id_type, dof_id_type>, std::vector<dof_id_type>> edge_to_elems;
  std::vector<dof_id_type> sliver_ids;
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    if (elem->type() != TRI3)
      continue;
    for (const auto s : make_range(3u))
      edge_to_elems[edge_key(elem->node_id(s), elem->node_id((s + 1) % 3))].push_back(elem->id());
    unsigned int lng;
    bool is_sliver;
    analyze(*elem, lng, is_sliver);
    if (is_sliver)
      sliver_ids.push_back(elem->id());
  }

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  std::unordered_set<dof_id_type> touched;
  std::size_t num_repaired = 0;

  // Repair each sliver present in the original mesh once. Triangles created by a split are never
  // re-examined, so this terminates and the surface stays watertight (each repair removes one
  // triangle and reshapes its neighbor into two).
  for (const auto sid : sliver_ids)
  {
    if (touched.count(sid))
      continue;
    Elem * s = mesh->elem_ptr(sid);
    if (!s)
      continue;
    unsigned int lng;
    bool is_sliver;
    analyze(*s, lng, is_sliver);
    if (!is_sliver)
      continue;

    Node * A = s->node_ptr(lng);
    Node * B = s->node_ptr((lng + 1) % 3);
    Node * C = s->node_ptr((lng + 2) % 3);

    // Find the TRI3 neighbor sharing the longest edge A-B
    Elem * neighbor = nullptr;
    for (const auto nid : libmesh_map_find(edge_to_elems, edge_key(A->id(), B->id())))
    {
      if (nid == sid || touched.count(nid))
        continue;
      Elem * cand = mesh->elem_ptr(nid);
      if (cand && cand->type() == TRI3)
      {
        neighbor = cand;
        break;
      }
    }
    // The longest edge is on a surface boundary (or its neighbor was already repaired): we cannot
    // heal the sliver by splitting, so leave it in place
    if (!neighbor)
      continue;

    // Local side of the neighbor that is the shared edge A-B, and its third (apex) node
    unsigned int eidx = libMesh::invalid_uint;
    for (const auto e : make_range(3u))
    {
      const auto a = neighbor->node_id(e), b = neighbor->node_id((e + 1) % 3);
      if ((a == A->id() && b == B->id()) || (a == B->id() && b == A->id()))
      {
        eidx = e;
        break;
      }
    }
    if (eidx == libMesh::invalid_uint)
      continue; // should not happen
    Node * e1 = neighbor->node_ptr((eidx + 1) % 3);
    Node * w = neighbor->node_ptr((eidx + 2) % 3);
    const subdomain_id_type sub = neighbor->subdomain_id();

    // Capture the boundary ids on the sides of both triangles, keyed by node-id pair, so we can put
    // them back on the surviving edges of the split
    std::map<std::pair<dof_id_type, dof_id_type>, std::vector<boundary_id_type>> edge_bcs;
    std::vector<boundary_id_type> side_ids;
    for (const Elem * t : {static_cast<const Elem *>(s), static_cast<const Elem *>(neighbor)})
      for (const auto e : make_range(3u))
      {
        boundary_info.boundary_ids(t, cast_int<unsigned short>(e), side_ids);
        if (!side_ids.empty())
        {
          auto & dst = edge_bcs[edge_key(t->node_id(e), t->node_id((e + 1) % 3))];
          dst.insert(dst.end(), side_ids.begin(), side_ids.end());
        }
      }
    boundary_info.remove(s);
    boundary_info.remove(neighbor);

    // Reshape the neighbor in place into (e0, C, w) and add the second half (C, e1, w); both keep
    // the neighbor's original winding
    neighbor->set_node((eidx + 1) % 3, C);
    auto half = std::make_unique<Tri3>();
    half->set_node(0, C);
    half->set_node(1, e1);
    half->set_node(2, w);
    half->subdomain_id() = sub;
    Elem * new_tri = mesh->add_elem(std::move(half));

    // Restore the captured boundary ids onto the matching edges of the two resulting triangles
    auto reassign = [&](Elem * t)
    {
      for (const auto e : make_range(3u))
      {
        auto it = edge_bcs.find(edge_key(t->node_id(e), t->node_id((e + 1) % 3)));
        if (it != edge_bcs.end())
          for (const auto bid : it->second)
            boundary_info.add_side(t, cast_int<unsigned short>(e), bid);
      }
    };
    reassign(neighbor);
    reassign(new_tri);

    mesh->delete_elem(s);
    touched.insert(sid);
    touched.insert(neighbor->id());
    ++num_repaired;
  }

  if (num_repaired)
  {
    mesh->prepare_for_use();
    _console << "Number of sliver triangles repaired: " << num_repaired << std::endl;
  }
}
