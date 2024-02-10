//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarsenBlockGenerator.h"
#include "MooseMeshUtils.h"
#include "MeshCoarseningUtils.h"
#include "MeshBaseDiagnosticsUtils.h"

#include "libmesh/elem.h"
#include "libmesh/mesh_modification.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", CoarsenBlockGenerator);

InputParameters
CoarsenBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which coarsens one or more blocks in an existing "
                             "mesh. The coarsening algorithm works best for regular meshes.");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to coarsen");
  params.addRequiredParam<std::vector<SubdomainName>>("block",
                                                      "The list of blocks to be coarsened");
  params.addRequiredParam<std::vector<unsigned int>>(
      "coarsening",
      "Maximum amount of times to coarsen elements in each block. See 'block' for indexing");
  params.addRequiredParam<Point>("starting_point",
                                 "A point inside the element to start the coarsening from");

  // This is a heuristic to be able to coarsen inside blocks that are not uniformly refined
  params.addRangeCheckedParam<Real>(
      "maximum_volume_ratio",
      2,
      "maximum_volume_ratio > 0",
      "Maximum allowed volume ratio between two fine elements to propagate "
      "the coarsening front through a side");
  params.addParam<bool>(
      "verbose",
      false,
      "Whether to make the mesh generator output details of its actions on the console");
  params.addParam<bool>("check_for_non_conformal_output_mesh",
                        true,
                        "Whether to check the entire mesh for non-conformal nodes indicating that "
                        "the coarsening operation has failed to produce a conformal mesh");
  return params;
}

CoarsenBlockGenerator::CoarsenBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _block(getParam<std::vector<SubdomainName>>("block")),
    _coarsening(getParam<std::vector<unsigned int>>("coarsening")),
    _starting_point(getParam<Point>("starting_point")),
    _max_vol_ratio(getParam<Real>("maximum_volume_ratio")),
    _verbose(getParam<bool>("verbose")),
    _check_output_mesh_for_nonconformality(getParam<bool>("check_for_non_conformal_output_mesh"))
{
  if (_block.size() != _coarsening.size())
    paramError("coarsening", "The blocks and coarsening parameter vectors should be the same size");
}

std::unique_ptr<MeshBase>
CoarsenBlockGenerator::generate()
{
  // Get the list of block ids from the block names
  const auto block_ids =
      MooseMeshUtils::getSubdomainIDs(*_input, getParam<std::vector<SubdomainName>>("block"));
  const std::set<SubdomainID> block_ids_set(block_ids.begin(), block_ids.end());

  // Check that the block ids/names exist in the mesh
  std::set<SubdomainID> mesh_blocks;
  _input->subdomain_ids(mesh_blocks);

  for (std::size_t i = 0; i < block_ids.size(); ++i)
    if (!MooseMeshUtils::hasSubdomainID(*_input, block_ids[i]))
      paramError("block",
                 "The block '",
                 getParam<std::vector<SubdomainName>>("block")[i],
                 "' was not found within the mesh");

  // Error if it has not been implemented for this element type
  for (const auto & elem : _input->active_subdomain_set_elements_ptr_range(block_ids_set))
    // Only types implemented
    if (elem->type() != QUAD4 && elem->type() != HEX8)
      paramError("block",
                 "The input mesh contains an unsupported element type '" +
                     Moose::stringify(elem->type()) + "' for coarsening in block " +
                     std::to_string(elem->subdomain_id()));

  // Take ownership of the mesh
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_serial())
    paramError("input", "Input mesh must not be distributed");

  // Find the element to start from
  auto start_elem = (*mesh->sub_point_locator())(_starting_point);

  // Check that the starting element choice: in the block with the most coarsening requested
  if (!start_elem)
    paramError("starting_point", "No element was found at that point");
  unsigned int max_c = *std::max_element(_coarsening.begin(), _coarsening.end());
  for (const auto i : index_range(block_ids))
    if (block_ids[i] == start_elem->subdomain_id() && _coarsening[i] != max_c)
      mooseError("The starting element must be in the block set to be coarsened the most.\n"
                 "Starting element is in block ",
                 start_elem->subdomain_id(),
                 " set to be coarsened ",
                 _coarsening[i],
                 " times but the max coarsening required is ",
                 max_c);

  // Determine how many times the coarsening will be used
  if (max_c > 0 && !mesh->is_prepared())
    // we prepare for use to make sure the neighbors have been found
    mesh->prepare_for_use();

  auto mesh_ptr = recursiveCoarsen(block_ids, mesh, _coarsening, max_c, /*step=*/0);

  // element neighbors are not valid
  if (max_c > 0)
    mesh_ptr->set_isnt_prepared();

  // flip elements as we were not careful to build them with a positive volume
  MeshTools::Modification::orient_elements(*mesh_ptr);

  // check that we are not returning a non-conformal mesh
  if (_check_output_mesh_for_nonconformality)
  {
    mesh_ptr->prepare_for_use();
    unsigned int num_nonconformal_nodes = 0;
    MeshBaseDiagnosticsUtils::checkNonConformalMesh(
        mesh_ptr, _console, 10, TOLERANCE, num_nonconformal_nodes);
    if (num_nonconformal_nodes)
      mooseError("Coarsened mesh has non-conformal nodes. The coarsening process likely failed to "
                 "form a uniform paving of coarsened elements. Number of non-conformal nodes: " +
                 Moose::stringify(num_nonconformal_nodes));
  }
  return mesh_ptr;
}

std::unique_ptr<MeshBase>
CoarsenBlockGenerator::recursiveCoarsen(const std::vector<subdomain_id_type> & block_ids,
                                        std::unique_ptr<MeshBase> & mesh,
                                        const std::vector<unsigned int> & coarsening,
                                        const unsigned int max,
                                        unsigned int coarse_step)
{
  if (coarse_step == max)
    return dynamic_pointer_cast<MeshBase>(mesh);

  // Elements should know their neighbors
  if (!mesh->is_prepared())
    mesh->prepare_for_use();

  // We wont be modifying the starting mesh for simplicity, we will make a copy and return that
  std::unique_ptr<MeshBase> mesh_return;
  int max_num_coarsened = -1;

  const auto base_start_elem = (*mesh->sub_point_locator())(_starting_point);

  // Try every node as the 'center' point of a coarsened element
  for (const auto & start_node_index : base_start_elem->node_index_range())
  {
    if (_verbose)
      _console << "Step " << coarse_step + 1 << " coarsening attempt #" << start_node_index
               << "\nUsing node " << *base_start_elem->node_ptr(start_node_index)
               << " as the interior node of the coarse element." << std::endl;

    // Make a copy of the mesh in case the initial node choice was bad
    auto mesh_copy = mesh->clone();

    // We will only have a single starting element for now. If there are non-connected components,
    // we will need to have a starting element-node pair in every component.
    auto start_elem = mesh_copy->elem_ptr(base_start_elem->id());
    mooseAssert(start_elem, "Should have a real elem pointer");
    mooseAssert(start_elem->active(), "Starting element must be active");

    auto start_node = start_elem->node_ptr(start_node_index);
    mooseAssert(start_node, "Starting node should exist");

    // Create comparator for ordering of candidates
    auto cmp = [](std::pair<Elem *, Node *> a, std::pair<Elem *, Node *> b)
    {
      // Sweep direction
      // Potentially a user selectable parameter in the future
      Point sorting_direction(1, 1, 1);
      const auto sorting =
          (a.first->vertex_average() - b.first->vertex_average()) * sorting_direction;
      if (MooseUtils::absoluteFuzzyGreaterThan(sorting, 0))
        return true;
      else if (MooseUtils::absoluteFuzzyEqual(sorting, 0) &&
               MooseUtils::absoluteFuzzyGreaterThan((*a.second - *b.second) * sorting_direction, 0))
        return true;
      else
        // Sorting direction is orthogonal to the two pairs, rely on element ids
        return a.first->id() > b.first->id();
    };

    // This set will keep track of all the 'fine elem' + 'coarse element interior node' pairs
    // we should attempt to form coarse element from
    // TODO: think about the implications of set vs vector. Set might grow to the entire mesh
    // due to sorting. Vector we could insert at the beginning and treat new candidates immediately
    std::set<std::pair<Elem *, Node *>, decltype(cmp)> candidate_pairs(cmp);
    candidate_pairs.insert(std::make_pair(start_elem, start_node));

    // Keep track of the coarse elements created
    std::set<Elem *> coarse_elems;

    while (candidate_pairs.size() > 0)
    {
      Elem * current_elem = candidate_pairs.begin()->first;
      Node * interior_node = candidate_pairs.begin()->second;
      mooseAssert(current_elem, "Null candidate element pointer");
      mooseAssert(interior_node, "Null candidate node pointer");
      const auto current_node_index = current_elem->get_node_index(interior_node);
      // just take any another node for now
      const auto ref_node =
          current_elem->node_ptr(current_node_index == 0 ? 1 : current_node_index - 1);
      mooseAssert(ref_node, "Should have a real node pointer");
      candidate_pairs.erase(candidate_pairs.begin());

      const auto elem_type = current_elem->type();

      // Mid-edge nodes could be an option too for making coarse elements.
      // For a first implementation, we wont try to use them as near the edge we would need
      // a special treatment.
      if (!current_elem->is_vertex(current_node_index))
        continue;

      // We dont support coarsening libMesh h-refined meshes
      if (current_elem->level() > 0)
        mooseError("H-refined meshes cannot be coarsened with this mesh generator. Use the "
                   "[Adaptivity] block to coarsen them.");

      // Get the nodes to build a coarse element
      std::vector<const Node *> tentative_coarse_nodes;
      std::set<const Elem *> fine_elements_const;
      bool success = MeshCoarseningUtils::getFineElementsFromInteriorNode(
          *interior_node, *ref_node, *current_elem, tentative_coarse_nodes, fine_elements_const);

      // For example, not enough fine elements around the node to build a coarse element
      if (!success)
        continue;

      bool go_to_next_candidate = false;
      // If the fine elements are not all of the same type, we currently cannot coarsen
      for (auto elem : fine_elements_const)
        if (elem && elem->type() != elem_type)
          go_to_next_candidate = true;

      // We do not coarsen across subdomains for now
      const auto common_subdomain_id = current_elem->subdomain_id();
      for (auto elem : fine_elements_const)
        if (elem && elem->subdomain_id() != common_subdomain_id)
          go_to_next_candidate = true;

      // Check the coarse element nodes gathered
      for (const auto & check_node : tentative_coarse_nodes)
        if (check_node == nullptr)
          go_to_next_candidate = true;

      if (go_to_next_candidate)
        continue;

      // We will likely delete the fine elements so we have to drop the const
      auto cmp_elem = [](Elem * a, Elem * b) { return a->id() - b->id(); };
      std::set<Elem *, decltype(cmp_elem)> fine_elements(cmp_elem);
      for (const auto elem_ptr : fine_elements_const)
        fine_elements.insert(mesh_copy->elem_ptr(elem_ptr->id()));

      // Form a parent, of a low order type as we only have the extreme vertex nodes
      std::unique_ptr<Elem> parent = Elem::build(Elem::first_order_equivalent_type(elem_type));
      parent->subdomain_id() = common_subdomain_id;
      auto parent_ptr = mesh_copy->add_elem(parent.release());
      coarse_elems.insert(parent_ptr);

      // Set the nodes to the coarse element
      // They were sorted previously in getFineElementFromInteriorNode
      for (auto i : index_range(tentative_coarse_nodes))
        parent_ptr->set_node(i) = mesh_copy->node_ptr(tentative_coarse_nodes[i]->id());

      // Gather targets / next candidates for the next element coarsening
      // Find the face neighbors, then look for the center node
      for (const auto side_index : make_range(parent_ptr->n_sides()))
      {
        // Pick one of the coarse element nodes by that face
        // it should not matter which one, they are all vertex nodes of a fine element
        // that has a neighbor on the other side of the coarse element face
        const auto coarse_node = parent_ptr->side_ptr(side_index)->node_ptr(0);
        mooseAssert(coarse_node,
                    "We should have a node on coarse side " + std::to_string(side_index));

        // Find one of the fine elements next to the face, its neighbor on the other side
        // of the coarse face is the face neighbor we want
        Elem * fine_el = nullptr;
        for (const auto & fine_elem : fine_elements)
        {
          bool found = false;
          for (const auto & fine_elem_node : fine_elem->node_ref_range())
            if (MooseUtils::absoluteFuzzyEqual((*coarse_node - fine_elem_node).norm_sq(), 0))
            {
              fine_el = fine_elem;
              found = true;
              break;
            }
          if (found)
            break;
        }
        mooseAssert(fine_el, "We should have found a fine element for the next candidate");
        const Real fine_el_volume = fine_el->volume();

        // Get the element(s) on the other side of the coarse face
        // We can tentatively support three cases:
        // - 1 element on the other side, coarse as well (towards less refinement).
        //   In that case, do not do anything. Two coarse elements sitting next to each other is
        //   perfect. We can detect this case by looking at the element volumes, with a heuristic
        //   on the ratio of volumes
        // - same number of elements on the other side than the fine elements touching the face
        //   (refinement was uniform on both sides of the face, we have coarsened one side so far)
        // - more elements on the other side than the fine elements touching the face
        //   (more refinement on that side of the face initially, we are now two levels of
        //   refinement away)
        // TODO: That last case
        unsigned int fine_side_index = 0;
        const auto coarse_side_center = parent_ptr->side_ptr(side_index)->vertex_average();
        Real min_distance = std::numeric_limits<Real>::max();
        // There might be a better way to find this index. Smallest distance should work
        for (const auto side_index : make_range(fine_el->n_sides()))
        {
          // only two sides (quad), three sides (hex) also own the coarse node
          if (fine_el->side_ptr(side_index)->get_node_index(coarse_node) == libMesh::invalid_uint)
            continue;
          const auto dist =
              (fine_el->side_ptr(side_index)->vertex_average() - coarse_side_center).norm_sq();
          if (min_distance > dist)
          {
            min_distance = dist;
            fine_side_index = side_index;
          }
        }
        mooseAssert(min_distance != std::numeric_limits<Real>::max(),
                    "We should have found a side");

        // We cannot use the neighbor pointer from the fine element, or else wont be able to
        // deal with non-conformal meshes that are disjoint at this location
        // Instead we offset a little and use a point locator
        Point offset_point =
            fine_el->side_ptr(fine_side_index)->vertex_average() +
            100 * TOLERANCE *
                (fine_el->side_ptr(fine_side_index)->vertex_average() - fine_el->vertex_average());
        auto pl = mesh_copy->sub_point_locator();
        pl->enable_out_of_mesh_mode();
        auto const_neighbor = (*pl)(offset_point);
        pl->disable_out_of_mesh_mode();

        // We're at a boundary
        if (!const_neighbor)
          continue;

        // Get a non-const element since it will be a candidate for deletion
        auto neighbor_fine_elem = mesh_copy->elem_ptr(const_neighbor->id());

        // Point locator finding a fine element inside
        if (fine_elements.find(neighbor_fine_elem) != fine_elements.end())
          continue;

        // Get the interior node for the next tentative coarse element
        // We can just use the index to get it from the next tentative fine element
        const auto neighbor_coarse_node_index = neighbor_fine_elem->get_node_index(coarse_node);
        // Node is not shared between the coarse element and its fine neighbors.
        // The mesh should probably be stitched before attempting coarsening
        if (neighbor_coarse_node_index == libMesh::invalid_uint)
        {
          mooseInfoRepeated("Coarse element node " + Moose::stringify(*coarse_node) +
                            " does not seem shared with any element other than the coarse element. "
                            "Is the mesh will stitched? Or are there non-conformalities?");
          continue;
        }
        const auto opposite_node_index = MeshCoarseningUtils::getOppositeNodeIndex(
            neighbor_fine_elem->type(), neighbor_coarse_node_index);
        auto neighbor_interior_node = neighbor_fine_elem->node_ptr(opposite_node_index);

        // avoid attempting to coarsen again an element we've already coarsened
        if (coarse_elems.find(neighbor_fine_elem) == coarse_elems.end())
        {
          // dont add a candidate if it's too early to coarsen it (will be coarsened on next step)
          // dont add a candidate if they are close to the size of a coarsened element already
          for (const auto i : index_range(block_ids))
            if (block_ids[i] == neighbor_fine_elem->subdomain_id() &&
                coarsening[i] > max - coarse_step - 1 &&
                std::abs(neighbor_fine_elem->volume()) < std::abs(_max_vol_ratio * fine_el_volume))
            {
              candidate_pairs.insert(std::make_pair(neighbor_fine_elem, neighbor_interior_node));
              break;
            }
        }
      }

      // Delete the elements used to build the coarse element
      for (auto & fine_elem : fine_elements)
      {
        if (!fine_elem)
          continue;

        // We dont delete nodes in the fine elements as they get removed during renumbering/
        // remove_orphaned_nodes calls in preparing for use
        mesh_copy->delete_elem(fine_elem);

        // Clean up the list of candidates from any deleted elements
        for (auto iter = candidate_pairs.begin(); iter != candidate_pairs.end();)
        {
          if (iter->first == fine_elem)
          {
            iter = candidate_pairs.erase(iter);
            continue;
          }
          ++iter;
        }
      }

      // Contract to remove the elements that were marked for deletion
      mesh_copy->contract();
      // Prepare for use to refresh the element neighbors
      mesh_copy->prepare_for_use();
    }

    // We pick the configuration (eg starting node) for which we managed to coarsen the most
    // This isn't the best idea, as some coarsening could be invalid (non-conformalities)
    // Maybe we should examine for non-conformality here to make a decision?
    // It's expensive to do so in a global mesh-wide check though, maybe if we baked that check
    // into the coarsening work it would be more reasonable.
    if (_verbose)
      _console << "Step " << coarse_step + 1 << " attempt #" << start_node_index << " created "
               << coarse_elems.size() << " coarse elements." << std::endl;
    if (int(coarse_elems.size()) > max_num_coarsened)
    {
      mesh_return = std::move(mesh_copy);
      max_num_coarsened = coarse_elems.size();
    }
  }
  if (_verbose)
    _console << "Step " << coarse_step + 1 << " created " << max_num_coarsened
             << " coarsened elements in its most successful attempt." << std::endl;
  coarse_step++;
  return recursiveCoarsen(block_ids, mesh_return, coarsening, max, coarse_step);
}
