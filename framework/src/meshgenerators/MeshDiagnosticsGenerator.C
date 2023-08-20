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
      std::cout << "Detected " << num_close_elems << " elements close to node " << *node
                << std::endl;

      // Keep track of the refined elements and the coarse element
      std::set<const Elem *> elements;
      std::set<const Elem *> coarse_elements;

      // loop through the set of elements near this node
      for (auto elem : elements_around)
      {
        // If the node is not part of this element's nodes, it is a
        // case of non-conformality
        bool node_on_elem = false;

        for (auto & elem_node : elem->node_ref_range())
        {
          if (*node == elem_node)
          {
            node_on_elem = true;
            break;
          }
        }
        if (node_on_elem)
          elements.insert(elem);
        // Else, the node is not part of the element considered, so if the element had been part of
        // an AMR-created non-conformality, this element is on the coarse side
        if (!node_on_elem)
          coarse_elements.insert(elem);
      }

      std::cout << elements.size() << elements_around.size() << coarse_elements.size() << std::endl;

      // all the elements around contained the node as one of their nodes
      if (elements.size() == elements_around.size())
        continue;

      // only one coarse element in front of refined elements. Whatever we're looking at is
      // not the interface between coarse and refined elements
      if (coarse_elements.size() > 1)
        continue;

      std::cout << "here" << std::endl;

      // Depending on the type of element, we already know the number of elements we expect
      // to be part of this set of likely refined candidates for a given non-conformal node to
      // examine. We can only decide if it was born out of AMR if it's the center node of the face
      // of a coarse element near refined elements
      auto elem_type = (*elements.begin())->type();
      if ((elem_type == HEX8 || elem_type == HEX20 || elem_type == HEX27) && elements.size() != 4)
        continue;
      if ((elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9) && elements.size() != 2)
        continue;
      if ((elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7) && elements.size() != 3)
        continue;
      if ((elem_type == TET4 || elem_type == TET10 || elem_type == TET14) && elements.size() != 4)
        continue;

      if (elements.empty())
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

      if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9)
      {
        auto elem = *elements.begin();
        tentative_coarse_nodes.resize(4);
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
        // std::cout << "interior node at " << *interior_node << std::endl;

        // Add point neighbors of interior node to list of potentially refined elements
        elem->find_point_neighbors(*interior_node, elements);

        // We'll need to order the coarse nodes based on the clock-wise order of the elements
        // Define a frame in which to compute the angles of the elements centers
        // angle 0 is node - interior node
        auto start_clock = *node - *interior_node;
        auto normal = (elem->vertex_average() - *interior_node).cross(start_clock);
        start_clock /= start_clock.norm();
        normal /= normal.norm();

        std::vector<std::pair<unsigned int, Real>> elements_angles(elements.size());
        unsigned int angle_i = 0;
        for (const auto elem_around : elements)
        {
          auto vec = elem_around->vertex_average() - *interior_node;
          vec /= vec.norm();
          auto angle = atan2(vec.cross(start_clock) * normal, vec * start_clock);
          elements_angles[angle_i] = std::make_pair(angle_i, angle);
          angle_i++;
        }

        // sort by angle, so it goes around the interior node
        std::sort(elements_angles.begin(),
                  elements_angles.end(),
                  [](auto & left, auto & right) { return left.second < right.second; });
        std::vector<unsigned int> elements_order(4);
        unsigned int order_i = 0;
        for (const auto & angle_pair : elements_angles)
        {
          // std::cout << "Node " << angle_pair.first << " angle " << angle_pair.second << "
          // order
          // "
          //           << order_i << std::endl;
          elements_order[angle_pair.first] = order_i;
          order_i++;
        }

        // The exterior nodes are the opposite nodes of the interior_node!
        unsigned int neighbor_i = 0;
        for (auto neighbor : elements)
        {
          const auto interior_node_number = neighbor->get_node_index(interior_node);
          unsigned int opposite_node_index = (interior_node_number + 2) % 4;

          // std::cout << *neighbor->node_ptr(opposite_node_index) << std::endl;
          tentative_coarse_nodes[elements_order[neighbor_i++]] =
              neighbor->node_ptr(opposite_node_index);
        }
      }
      // For TRI elements, there is an element at the center
      else if (elem_type == TRI3 || elem_type == TRI6 || elem_type == TRI7)
      // || elem_type == TET4 ||
      //       elem_type == TET10 || elem_type == TET14)
      {
        std::cout << "Building here" << std::endl;
        // Find the center element
        // It's the only element that does not share a node with the coarse element
        const auto coarse_element = *coarse_elements.begin();
        const Elem * center_elem;
        for (const auto refined_elem : elements)
        {
          bool shares_a_node_with_coarse = false;
          for (const auto & other_node : refined_elem->node_ref_range())
            for (const auto & coarse_node : coarse_element->node_ref_range())
              if (other_node == coarse_node)
              {
                shares_a_node_with_coarse = true;
                tentative_coarse_nodes.push_back(&other_node);
              }
          if (!shares_a_node_with_coarse)
            center_elem = refined_elem;
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
          if (center_elem->side_ptr(center_side_opposite_node)->get_node_index(&tri_node) ==
              libMesh::invalid_uint)
            tentative_coarse_nodes.push_back(&tri_node);

        mooseAssert(tentative_coarse_nodes.size() == 3,
                    "We are forming a coarsened triangle element");
      }

      // Check the element types: if not all the same then it's not uniform AMR
      for (auto elem : elements)
        if (elem->type() != elem_type)
          continue;

      std::cout << "Nodes to build the parent: " << tentative_coarse_nodes.size() << std::endl;

      // Check the number of coarse element nodes gathered
      for (auto node : tentative_coarse_nodes)
        if (node == nullptr)
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
      {
        std::cout << "Node around " << *tentative_coarse_nodes[i] << std::endl;
        parent->set_node(i) = const_cast<Node *>(tentative_coarse_nodes[i]);
      }

      // Refine this parent
      parent->set_refinement_flag(Elem::REFINE);

      MeshRefinement mesh_refiner(*mesh);
      parent->refine(mesh_refiner);
      std::cout << "Child " << parent->n_children() << std::endl;

      // Compare with the original set of elements
      // We already know the child share the exterior node. If they share the same vertex
      // average as the group of unrefined elements we will call this good enough for now
      bool all_children_match = true;
      for (auto & child : parent->child_ref_range())
      {
        bool found_child = false;
        std::cout << "Center of child " << child.vertex_average() << " " << std::endl;
        for (auto & potential_children : elements)
          if (MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(0), potential_children->vertex_average()(0), TOLERANCE) &&
              MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(1), potential_children->vertex_average()(1), TOLERANCE) &&
              MooseUtils::absoluteFuzzyEqual(
                  child.vertex_average()(2), potential_children->vertex_average()(2), TOLERANCE))
          {
            found_child = true;
            break;
          }
        if (!found_child)
        {
          all_children_match = false;
          break;
        }
      }

      if (all_children_match)
      {
        num_likely_AMR_created_nonconformality++;
        if (num_likely_AMR_created_nonconformality < 10)
          _console << "Detected non-conformality likely created by AMR near " << *node << std::endl;
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
                                         bool may_error)
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
