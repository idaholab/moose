//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshDiagnosticsGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/mesh_refinement.h"

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
  params.addParam<Real>("nonconformal_tol", 1e-8, "tolerance for element non-conformality");
  params.addParam<MooseEnum>(
      "search_for_adaptivity_nonconformality",
      chk_option,
      "whether to check for non-conformality arising from adaptive mesh refinement");
  return params;
}

MeshDiagnosticsGenerator::MeshDiagnosticsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _check_element_volumes(getParam<MooseEnum>("examine_element_volumes")),
    _min_volume(getParam<Real>("minimum_element_volumes")),
    _max_volume(getParam<Real>("maximum_element_volumes")),
    _check_element_types(getParam<MooseEnum>("examine_element_types")),
    _check_element_overlap(getParam<MooseEnum>("examine_element_overlap")),
    _check_non_planar_sides(getParam<MooseEnum>("examine_nonplanar_sides")),
    _check_non_conformal_mesh(getParam<MooseEnum>("examine_non_conformality")),
    _non_conformality_tol(getParam<Real>("nonconformal_tol")),
    _check_adaptivity_non_conformality(getParam<MooseEnum>("search_for_adaptivity_nonconformality"))
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
}

std::unique_ptr<MeshBase>
MeshDiagnosticsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Most of the checks assume we use a replicated mesh
  if (!mesh->is_replicated())
    mooseError("Only replicated meshes are supported");

  // We prepare for use at the beginning to facilitate diagnosis
  // This deliberately does not trust the mesh to know whether it's already prepared or not
  mesh->prepare_for_use();

  if (_check_element_volumes != "NO_CHECK")
  {
    // loop elements within the mesh (assumes replicated)
    for (auto & elem : mesh->active_element_ptr_range())
    {
      if (elem->volume() <= _min_volume)
      {
        if (_num_tiny_elems < 10)
          _console << "Element too small detected with centroid : " << elem->true_centroid()
                   << std::endl;
        else if (_num_tiny_elems == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
        _num_tiny_elems++;
      }
      if (elem->volume() >= _max_volume)
      {
        if (_num_big_elems < 10)
          _console << "Element too large detected with centroid : " << elem->true_centroid()
                   << std::endl;
        else if (_num_big_elems == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
        _num_big_elems++;
      }
    }
    diagnosticsLog("Number of elements below prescribed volume : " +
                       std::to_string(_num_tiny_elems),
                   _check_element_volumes,
                   _num_tiny_elems);
    diagnosticsLog("Number of elements above prescribed volume : " + std::to_string(_num_big_elems),
                   _check_element_volumes,
                   _num_big_elems);
  }

  if (_check_element_types != "NO_CHECK")
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
      {
        types.insert(elem->type());
      }
      std::string elem_type_names;
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

  if (_check_element_overlap != "NO_CHECK")
  {
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

        bool found = false;
        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            found = true;
            break;
          }
        }
        if (!found)
        {
          _num_elem_overlaps++;
          if (_num_elem_overlaps < 10)
            _console << "Element overlap detected at : " << *node << std::endl;
          else if (_num_elem_overlaps == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }

    diagnosticsLog("Number of elements overlapping (node-based heuristics): " +
                       Moose::stringify(_num_elem_overlaps),
                   _check_element_overlap,
                   _num_elem_overlaps);
    _num_elem_overlaps = 0;

    // loop on all elements in mesh: assumes a replicated mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      // find all the elements around the centroid of this element
      std::set<const Elem *> overlaps;
      (*pl)(elem->vertex_average(), overlaps);

      if (overlaps.size() > 1)
      {
        _num_elem_overlaps++;
        if (_num_big_elems < 10)
          _console << "Element overlap detected at a centroid : " << elem->vertex_average()
                   << std::endl;
        else if (_num_elem_overlaps == 10)
          _console << "Maximum output reached, log is silenced" << std::endl;
      }
    }
    diagnosticsLog("Number of elements overlapping (centroid-based heuristics): " +
                       Moose::stringify(_num_elem_overlaps),
                   _check_element_overlap,
                   _num_elem_overlaps);
  }

  if (_check_non_planar_sides != "NO_CHECK")
  {
    // loop on all elements in mesh: assumes a replicated mesh
    for (auto & elem : mesh->active_element_ptr_range())
    {
      for (auto i : make_range(elem->n_sides()))
      {
        auto side = elem->side_ptr(i);
        std::vector<Point *> nodes;
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

        for (auto in : make_range(nodes.size() - 3))
        {
          RealVectorValue v3 = *nodes[0] - *nodes[in + 3];
          bool planar = MooseUtils::absoluteFuzzyEqual(v2.cross(v1) * v3, 0);
          if (!planar)
            found_non_planar = true;
        }

        if (found_non_planar)
        {
          _sides_non_planar++;
          if (_sides_non_planar < 10)
            _console << "Nonplanar side detected at :" << elem->side_ptr(i)->vertex_average()
                     << std::endl;
          else if (_sides_non_planar == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }
    diagnosticsLog("Number of non-planar element sides detected: " +
                       Moose::stringify(_sides_non_planar),
                   _check_non_planar_sides,
                   _sides_non_planar);
  }

  if (_check_non_conformal_mesh != "NO_CHECK")
  {
    auto pl = mesh->sub_point_locator();
    // loop on nodes, assumes a replicated mesh
    for (auto & node : mesh->node_ptr_range())
    {
      pl->set_close_to_point_tol(_non_conformality_tol);
      // find all the elements around this node
      std::set<const Elem *> elements;
      (*pl)(*node, elements);

      // loop through the set of elements near this node
      for (auto & elem : elements)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool found_conformal = false;

        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            found_conformal = true;
            break;
          }
        }
        if (!found_conformal)
        {
          _num_nonconformal_nodes++;
          if (_num_nonconformal_nodes < 10)
            _console << "Non-conformality detected at  : " << *node << std::endl;
          else if (_num_nonconformal_nodes == 10)
            _console << "Maximum output reached, log is silenced" << std::endl;
        }
      }
    }
    diagnosticsLog("Number of non-conformal nodes: " + Moose::stringify(_num_nonconformal_nodes),
                   _check_non_conformal_mesh,
                   _num_nonconformal_nodes);
    pl->unset_close_to_point_tol();
  }

  if (_check_adaptivity_non_conformality != "NO_CHECK")
  {
    unsigned int num_likely_AMR_created_nonconformality = 0;
    auto pl = mesh->sub_point_locator();
    pl->set_close_to_point_tol(_non_conformality_tol);

    // loop on nodes, assumes a replicated mesh
    for (auto & node : mesh->node_ptr_range())
    {
      // find all the elements around this node
      std::set<const Elem *> elements_around;
      (*pl)(*node, elements_around);
      const auto num_close_elems = elements_around.size();

      // Keep track of the refined elements and the coarse element
      std::set<const Elem *> elements;
      std::set<const Elem *> coarse_elements;

      // loop through the set of elements near this node
      for (auto elem : elements_around)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool node_on_elem = false;

        // non-vertex nodes are not cause for the kind of non-conformality we are looking for
        if (elem->get_node_index(node) != libMesh::invalid_uint &&
            elem->is_vertex(elem->get_node_index(node)))
          node_on_elem = true;

        if (node_on_elem)
          elements.insert(elem);
        // Else, the node is not part of the element considered, so if the element had been part of
        // an AMR-created non-conformality, this element is on the coarse side
        if (!node_on_elem)
          coarse_elements.insert(elem);
      }

      // all the elements around contained the node as one of their nodes
      // if the coarse and refined sides are not stitched together, this check can fail,
      // as nodes that are physically near one element are not part of it because of the lack of
      // stitching (overlapping nodes)
      if (elements.size() == elements_around.size())
        continue;

      if (elements.empty())
        continue;

      // Depending on the type of element, we already know the number of elements we expect
      // to be part of this set of likely refined candidates for a given non-conformal node to
      // examine. We can only decide if it was born out of AMR if it's the center node of the face
      // of a coarse element near refined elements
      auto elem_type = (*elements.begin())->type();
      if ((elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9) && elements.size() != 2)
        continue;
      if ((elem_type == HEX8 || elem_type == HEX20 || elem_type == HEX27) && elements.size() != 4)
        continue;
      if ((elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7) && elements.size() != 3)
        continue;
      if ((elem_type == TET4 || elem_type == TET10 || elem_type == TET14) && elements.size() != 8)
        continue;

      // only one coarse element in front of refined elements except for tets. Whatever we're
      // looking at is not the interface between coarse and refined elements
      // Tets are split on their edges (rather than the middle of a face) so there could be any
      // number of coarse elements in front of the node non-conformality created by refinement
      if (elem_type != TET4 && elem_type != TET10 && elem_type != TET14 &&
          coarse_elements.size() > 1)
        continue;

      // There exists non-conformality, the node should have been a node of all the elements
      // that are close enough to the node, and it is not

      // For quads and hexes, there is one (quad) or four (hexes) sides that are tied to this node
      // at the non-conformal interface between the refined elements and a coarse element
      unsigned int side_inside_parent;
      if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9 || elem_type == HEX8 ||
          elem_type == HEX20 || elem_type == HEX27)
      {
        auto elem = *elements.begin();
        // Find which sides (of the elements) the node considered is part of
        std::vector<Elem *> node_on_sides;
        for (auto i : make_range(elem->n_sides()))
        {
          auto side = elem->side_ptr(i);
          std::vector<const Node *> other_nodes;
          bool node_on_side = false;
          for (const auto & elem_node : side->node_ref_range())
          {
            if (*node == elem_node)
              node_on_side = true;
            else
              other_nodes.emplace_back(node);
          }
          // node is on the side, but is it the side that goes away from the coarse element
          if (node_on_side)
          {
            // if all the other nodes on this side are in one of the other potentially refined
            // elements, it's one of the side(s) (4 sides in a 3D hex for example) inside the
            // parent
            for (auto other_elem : elements)
            {
              bool all_other_nodes_on_shared_side = true;
              for (const auto & other_node : other_nodes)
                if (other_elem->get_node_index(other_node) == libMesh::invalid_uint)
                  all_other_nodes_on_shared_side = false;

              if (all_other_nodes_on_shared_side)
                side_inside_parent = i;
            }
          }
        }
      }

      // Nodes of the tentative parent element
      std::vector<const Node *> tentative_coarse_nodes;

      if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9 || elem_type == HEX8 ||
          elem_type == HEX20 || elem_type == HEX27)
      {
        auto elem = *elements.begin();
        // Gather the other potential elements in the refined element:
        // they are point neighbors of the node that is shared between all the elements flagged
        // for the non-conformality
        // Find shared node
        auto interior_side = elem->side_ptr(side_inside_parent);
        const Node * interior_node;
        for (const auto & other_node : interior_side->node_ref_range())
        {
          if (other_node == *node)
            continue;
          bool in_all_node_neighbor_elements = true;
          for (auto other_elem : elements)
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

        // Add point neighbors of interior node to list of potentially refined elements
        std::set<const Elem *> all_elements;
        elem->find_point_neighbors(*interior_node, all_elements);

        if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9)
        {
          // We need to order the fine elements so when we get the coarse element nodes they form a
          // non-twisted element
          tentative_coarse_nodes.resize(4);

          // The exterior nodes are the opposite nodes of the interior_node!
          unsigned int neighbor_i = 0;
          for (auto neighbor : all_elements)
          {
            const auto interior_node_number = neighbor->get_node_index(interior_node);
            unsigned int opposite_node_index = (interior_node_number + 2) % 4;

            tentative_coarse_nodes[neighbor_i++] = neighbor->node_ptr(opposite_node_index);
          }

          // Re-order nodes so that they will form a decent quad
          Point axis = (elem->vertex_average() - *interior_node).cross(*interior_node - *node);
          reorderNodes(tentative_coarse_nodes, interior_node, node, axis);
        }
        else
        {
          // Get the coarse neighbor side to be able to recognize nodes that should become part of
          // the coarse parent
          const auto & coarse_elem = *coarse_elements.begin();
          unsigned short coarse_side_i;
          for (const auto & coarse_side_index : coarse_elem->side_index_range())
          {
            const auto coarse_side_ptr = coarse_elem->side_ptr(coarse_side_index);
            // The side of interest is the side that contains the non-conformality
            if (!coarse_side_ptr->close_to_point(*node, 10 * TOLERANCE))
              continue;
            else
            {
              coarse_side_i = coarse_side_index;
              break;
            }
          }
          const auto coarse_side = coarse_elem->side_ptr(coarse_side_i);

          // We did not find the side of the coarse neighbor near the refined elements
          // It could be that it's not planar and the fine elements nodes do not lie on it
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
          for (const auto & elem_1 : elements)
            for (const auto & coarse_node : elem_1->node_ref_range())
            {
              bool node_shared = false;
              for (const auto & elem_2 : elements)
              {
                if (elem_2 != elem_1)
                  if (elem_2->get_node_index(&coarse_node) != libMesh::invalid_uint)
                    node_shared = true;
              }
              // A node for the coarse parent will appear in only one fine neighbor
              // and will lay on the side of the coarse neighbor
              if (!node_shared && coarse_side->close_to_point(coarse_node, TOLERANCE))
                tentative_coarse_nodes[i++] = &coarse_node;
              mooseAssert(i <= 5, "We went too far in this index");
            }

          // Need to order these nodes to form a valid quad / base of an hex
          // We go around the axis formed by the node and the interior node
          Point axis = *interior_node - *node;
          const auto start_circle = elem->vertex_average();
          reorderNodes(tentative_coarse_nodes, interior_node, &start_circle, axis);
          tentative_coarse_nodes.resize(8);

          // Use the neighbors of the fine elements that contain these nodes to get the vertex
          // nodes
          for (const auto & elem : elements)
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
              if (all_elements.count(neighbor) && !elements.count(neighbor))
              {
                // Find the coarse node for the neighbor
                const Node * coarse_elem_node;
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
              }
          }
        }

        // No need to separate fine elements near the non-conformal node and away from it
        elements = all_elements;
      }
      // For TRI elements, we use the fine triangle element at the center of the potential
      // coarse triangle element
      else if (elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7)
      {
        // Find the center element
        // It's the only element that shares a side with both of the other elements near the node
        // considered
        const Elem * center_elem;
        for (const auto refined_elem_1 : elements)
        {
          unsigned int num_neighbors = 0;
          for (const auto refined_elem_2 : elements)
          {
            if (refined_elem_1 == refined_elem_2)
              continue;
            if (refined_elem_1->has_neighbor(refined_elem_2))
              num_neighbors++;
          }
          if (num_neighbors >= 2)
            center_elem = refined_elem_1;
        }
        // Now get the tentative coarse element nodes
        for (const auto refined_elem : elements)
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
        unsigned int center_side_opposite_node;
        for (auto side_index : center_elem->side_index_range())
          if (center_elem->side_ptr(side_index)->get_node_index(node) == libMesh::invalid_uint)
            center_side_opposite_node = side_index;
        const auto neighbor_on_other_side_of_opposite_center_side =
            center_elem->neighbor_ptr(center_side_opposite_node);
        elements.insert(neighbor_on_other_side_of_opposite_center_side);
        for (const auto & tri_node :
             neighbor_on_other_side_of_opposite_center_side->node_ref_range())
          if (neighbor_on_other_side_of_opposite_center_side->is_vertex(
                  neighbor_on_other_side_of_opposite_center_side->get_node_index(&tri_node)) &&
              center_elem->side_ptr(center_side_opposite_node)->get_node_index(&tri_node) ==
                  libMesh::invalid_uint)
            tentative_coarse_nodes.push_back(&tri_node);

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
          for (const auto & elem : elements)
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
        for (const auto & elem : elements)
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
                if (tet_2 != neighbor &&
                    tet_2->get_node_index(&other_node) != libMesh::invalid_uint)
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
        elements.clear();
        for (const auto & elem : tips_tets)
          elements.insert(elem);
        for (const auto & elem : inside_tets)
          elements.insert(elem);

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
      }

      // Check the fine element types: if not all the same then it's not uniform AMR
      for (auto elem : elements)
        if (elem->type() != elem_type)
          continue;

      // Check the number of coarse element nodes gathered
      for (const auto & check_node : tentative_coarse_nodes)
        if (check_node == nullptr)
          continue;

      // Form a parent, of the same type as the elements we are trying to combine!
      std::unique_ptr<Elem> parent;
      // TODO clone instead of hard setting type
      if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9)
        parent = std::make_unique<Quad4>();
      if (elem_type == HEX8 || elem_type == HEX20 || elem_type == HEX27)
        parent = std::make_unique<Hex8>();
      if (elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7)
        parent = std::make_unique<Tri3>();
      if (elem_type == TET4 || elem_type == TET10 || elem_type == TET14)
        parent = std::make_unique<Tet4>();

      // Set the nodes to the coarse element
      for (auto i : index_range(tentative_coarse_nodes))
        parent->set_node(i) = const_cast<Node *>(tentative_coarse_nodes[i]);

      // Refine this parent
      parent->set_refinement_flag(Elem::REFINE);
      MeshRefinement mesh_refiner(*mesh);
      parent->refine(mesh_refiner);
      const auto num_children = parent->n_children();

      // Compare with the original set of elements
      // We already know the child share the exterior node. If they share the same vertex
      // average as the group of unrefined elements we will call this good enough for now
      // For tetradhedral elements we cannot rely on the children all matching as the choice in
      // the diagonal selection can be made differently. We'll just say 4 matching children is
      // good enough for the heuristic
      unsigned int num_children_match = 0;
      for (auto & child : parent->child_ref_range())
      {
        for (auto & potential_children : elements)
          if (MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(0), potential_children->vertex_average()(0), TOLERANCE) &&
              MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(1), potential_children->vertex_average()(1), TOLERANCE) &&
              MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(2), potential_children->vertex_average()(2), TOLERANCE))
          {
            num_children_match++;
            break;
          }
      }

      if (num_children_match == num_children || (parent->type() == TET4 && num_children_match == 4))
      {
        num_likely_AMR_created_nonconformality++;
        if (num_likely_AMR_created_nonconformality < 10)
        {
          _console << "Detected non-conformality likely created by AMR near" << *node
                   << Moose::stringify(elem_type)
                   << " elements that could be merged into a coarse element:" << std::endl;
          for (const auto & elem : elements)
            _console << elem->id() << " ";
          _console << std::endl;
        }
        else if (num_likely_AMR_created_nonconformality == 10)
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
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
MeshDiagnosticsGenerator::diagnosticsLog(std::string msg,
                                         const MooseEnum & log_level,
                                         bool may_error) const
{
  mooseAssert(log_level != "NO_CHECK",
              "We should not be outputting logs if the check had been disabled");
  if (log_level == "INFO" || !may_error)
    mooseInfoRepeated(msg);
  else if (log_level == "WARNING")
    mooseWarning(msg);
  else if (log_level == "ERROR")
    mooseError(msg);
  else
    mooseError("Should not reach here");
}

void
MeshDiagnosticsGenerator::reorderNodes(std::vector<const Node *> & nodes,
                                       const Point * origin,
                                       const Point * clock_start,
                                       Point & axis) const
{
  mooseAssert(axis.norm() != 0, "Invalid rotation axis when ordering nodes");
  mooseAssert(origin != clock_start, "Invalid starting direction when ordering nodes");

  // We'll need to order the coarse nodes based on the clock-wise order of the elements
  // Define a frame in which to compute the angles of the fine elements centers
  // angle 0 is the [interior node, non-conformal node] vertex
  auto start_clock = *origin - *clock_start;
  start_clock /= start_clock.norm();
  axis /= axis.norm();

  std::vector<std::pair<unsigned int, Real>> nodes_angles(nodes.size());
  for (const auto & angle_i : index_range(nodes))
  {
    auto vec = *nodes[angle_i] - *origin;
    vec /= vec.norm();
    const auto angle = atan2(vec.cross(start_clock) * axis, vec * start_clock);
    nodes_angles[angle_i] = std::make_pair(angle_i, angle);
  }

  // sort by angle, so it goes around the interior node
  std::sort(nodes_angles.begin(),
            nodes_angles.end(),
            [](auto & left, auto & right) { return left.second < right.second; });

  // Re-sort the nodes based on their angle
  std::vector<const Node *> new_nodes(nodes.size());
  for (const auto & old_index : index_range(nodes))
    new_nodes[old_index] = nodes[nodes_angles[old_index].first];
  for (const auto & index : index_range(nodes))
    nodes[index] = new_nodes[index];
}
