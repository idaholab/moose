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

#include "libmesh/elem.h"
#include "CastUniquePointer.h"

registerMooseObject("MooseApp", CoarsenBlockGenerator);

InputParameters
CoarsenBlockGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which coarsens one or more blocks in an existing mesh. The coarsening algorithm works best for regular meshes.");
  params.addRequiredParam<MeshGeneratorName>("input", "Input mesh to coarsen");
  params.addRequiredParam<std::vector<SubdomainName>>("block", "The list of blocks to be coarsened");
  params.addRequiredParam<std::vector<unsigned int>>(
      "coarsening",
      "Minimum amount of times to coarsen each block, corresponding to their index in 'block'");
  params.addRequiredParam<Point>("starting_point", "A point inside the element to start the coarsening from");
  params.addParam<bool>("verbose", false, "Whether to make the mesh generator output details of its actions on the console");
  return params;
}

CoarsenBlockGenerator::CoarsenBlockGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _block(getParam<std::vector<SubdomainName>>("block")),
    _coarsening(getParam<std::vector<unsigned int>>("coarsening")),
    _starting_point(getParam<Point>("starting_point")),
    _verbose(getParam<bool>("verbose"))
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
    if (elem->type() != QUAD4)
      paramError("block", "The input mesh contains an unsupported element type '" + Moose::stringify(elem->type()) + "' for coarsening in block " + std::to_string(elem->subdomain_id()));

  // Take ownership of the mesh
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Find the element to start from
  auto start_elem = (*mesh->sub_point_locator())(_starting_point);

  // Check that the starting element choice: in the block with the most coarsening requested
  if (!start_elem)
    paramError("starting_point", "No element was found at that point");
  unsigned int max_c = *std::max_element(_coarsening.begin(), _coarsening.end());
  for (const auto i : index_range(block_ids))
    if (block_ids[i] == start_elem->subdomain_id() && _coarsening[i] != max_c)
      mooseError("The starting element must be in the block set to be coarsened the most.\n"
                 "Starting element is in block ", start_elem->subdomain_id(), " set to be coarsened ", _coarsening[i], " times but the max coarsening required is ", max_c);

  // Determine how many times the coarsening will be used
  int max = *std::max_element(_coarsening.begin(), _coarsening.end());
  if (max > 0 && !mesh->is_prepared())
    // we prepare for use to make sure the neighbors have been found
    mesh->prepare_for_use();

  auto mesh_ptr = recursive_coarsen(block_ids, mesh, _coarsening, max, /*step=*/0);

  // element neighbors are not valid
  if (max > 0)
    mesh_ptr->set_isnt_prepared();

  return mesh_ptr;
}

std::unique_ptr<MeshBase>
CoarsenBlockGenerator::recursive_coarsen(const std::vector<subdomain_id_type> & block_ids,
                                         std::unique_ptr<MeshBase> & mesh,
                                         const std::vector<unsigned int> & coarsening,
                                         unsigned int max,
                                         unsigned int coarse_step)
{
  if (coarse_step == max)
    return dynamic_pointer_cast<MeshBase>(mesh);

  // We wont be modifying the starting mesh for simplicity, we will make a copy and return that
  std::unique_ptr<MeshBase> mesh_return;
  int max_num_coarsened = -1;

  const auto base_start_elem = (*mesh->sub_point_locator())(_starting_point);

  // Try every node as the 'center' point of a coarsened element
  for (const auto & start_node_index : base_start_elem->node_index_range())
  {
    if (_verbose)
      _console << "Step " << coarse_step + 1 << " coarsening attempt #" << start_node_index << "\nUsing node " << *base_start_elem->node_ptr(start_node_index) << " as the interior node of the coarse element." << std::endl;

    // Make a copy of the mesh in case the initial node choice was bad
    auto mesh_copy = mesh->clone();
    // TODO:delete! we need point neighbors
    mesh_copy->prepare_for_use();

    // We will only have a single starting element for now. If there are non-connected components,
    // we will need to have a starting element-node pair in every component.
    auto start_elem = mesh_copy->elem_ptr(base_start_elem->id());
    mooseAssert(start_elem, "Should have a real elem pointer");
    if (!start_elem->active())
      mooseError("Starting element must be active");

    auto start_node = start_elem->node_ptr(start_node_index);
    mooseAssert(start_node, "Should have a real node pointer");

    // This set will keep track of all the 'fine elem' + 'coarse element interior node' pairs
    // we should attempt to form coarse element from
    // TODO: think about the implications of set vs vector. Set might grow to the entire mesh
    // due to sorting. Vector we could insert at the beginning and treat new candidates immediately
    std::set<std::pair<Elem *, Node *>> candidate_pairs;
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
      // just take any another node for now FIXME
      const auto ref_node = current_elem->node_ptr(current_node_index == 0 ? 1 : current_node_index - 1);
      mooseAssert(ref_node, "Should have a real node pointer");
      _console << "Current number of candidates for coarsening " << candidate_pairs.size() << std::endl;
      std::cout << "Examining elem " << current_elem->id() << " and node " << interior_node->id() << std::endl;
      candidate_pairs.erase(candidate_pairs.begin());

      const auto elem_type = current_elem->type();

      // Mid-edge nodes could be an option too for making coarse elements.
      // For a first implementation, we wont try to use them as near the edge we would need
      // a special treatment.
      if (!current_elem->is_vertex(current_node_index))
      {
        std::cout << "Dismissing not a vertex " << std::endl;
        continue;
      }

      // Get the nodes to build a coarse element
      std::vector<const Node *> tentative_coarse_nodes;
      std::set<const Elem *> fine_elements_const;
      bool success = MeshCoarseningUtils::getFineElementFromInteriorNode(interior_node,
                                                          ref_node,
                                                          current_elem,
                                                          TOLERANCE,
                                                          tentative_coarse_nodes,
                                                          fine_elements_const);

      // For example, not enough fine elements around the node to build a coarse element
      if (!success)
      {
        std::cout << "Dismissing due to " << fine_elements_const.size() << std::endl;
        continue;
      }

      // We might delete them so we have to drop the const
      std::set<Elem *> fine_elements;
      for (const auto elem_ptr : fine_elements_const)
        fine_elements.insert(const_cast<Elem *>(elem_ptr));

      // If the fine elements are not all of the same type, we currently cannot coarsen
      for (auto elem : fine_elements)
        if (elem && elem->type() != elem_type)
          continue;
  
      // We dont support recovering more than a single level of libMesh h-refinement
      // and we dont support coarsening in a way that does not respect the libMesh coarse elements
      bool found_parent = false;
      bool parent_is_shared = true;
      Elem * common_parent;
      for (auto elem : fine_elements)
        if (elem)
        {
          if (!found_parent && elem->parent())
            common_parent = elem->parent();
          else if (elem->parent())
            if (elem->parent() != common_parent)
              parent_is_shared = false;
          mooseAssert(elem->level() < 2, "We did not implement more");
        }
      if (!parent_is_shared)
        continue;

      // Check the coarse element nodes gathered
      for (const auto & check_node : tentative_coarse_nodes)
        if (check_node == nullptr)
        {
          std::cout << "Dismissing bad node " << std::endl;
          continue;
        }

      // Form a parent, of a low order type as we only have the extreme vertex nodes
      std::unique_ptr<Elem> parent = Elem::build(Elem::first_order_equivalent_type(elem_type));
      auto parent_ptr = mesh_copy->add_elem(parent.release());
      coarse_elems.insert(parent_ptr);

      // Set the nodes to the coarse element
      for (auto i : index_range(tentative_coarse_nodes))
        parent_ptr->set_node(i) = mesh_copy->node_ptr(tentative_coarse_nodes[i]->id());

      // Gather targets for the next coarsening
      // Find the face neighbors, then look for the center node
      for (const auto side_index : make_range(parent_ptr->n_sides()))
      {
        // Pick one of the coarse element nodes by that face
        // it should not matter which one, they are all vertex nodes of a fine element
        // that has a neighbor on the other side of the coarse element face
        const auto coarse_node = parent_ptr->side_ptr(side_index)->node_ptr(0);
        mooseAssert(coarse_node, "We should have a node on coarse side " + std::to_string(side_index));

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
        std::cout << "Examining side of center " << parent_ptr->side_ptr(side_index)->vertex_average() << std::endl;
        std::cout << "Chose element " << fine_el->vertex_average() << std::endl;

        // Get the element(s) on the other side of the coarse face
        // We can tentatively support three cases:
        // - 1 element on the other side, coarse as well (towards less refinement).
        //   In that case, do not do anything. Two elements sitting next to each other is perfect
        // - same number of elements on the other side than the fine elements touching the face
        //   (refinement was uniform on both sides of the face, we have coarsened one side so far)
        // - more elements on the other side than the fine elements touching the face
        //   (more refinement on that side of the face initially, we are now two levels of refinement away)
        // TODO: That last case
        unsigned int fine_side_index = 0;
        const auto coarse_side_center = parent_ptr->side_ptr(side_index)->vertex_average();
        Real min_distance = std::numeric_limits<Real>::max();
        // There might be a better way to find this index. Smallest distance should work
        for (const auto side_index : make_range(fine_el->n_sides()))
        {
          if (fine_el->side_ptr(side_index)->get_node_index(coarse_node) == libMesh::invalid_uint)
            continue;
          const auto dist = (fine_el->side_ptr(side_index)->vertex_average() - coarse_side_center).norm_sq();
          if (min_distance > dist)
          {
            min_distance = dist;
            fine_side_index = side_index;
          }
        }
        mooseAssert(min_distance != std::numeric_limits<Real>::max(), "We should have found a side");
        std::cout << "Chose side " << fine_el->side_ptr(fine_side_index)->vertex_average() << std::endl;
        // We cannot use the neighbor pointer from the fine element, or else wont be able to
        // deal with non-conformal meshes that are disjoint at this location
        // Instead we offset a little and use a point locator
        Point offset_point = fine_el->side_ptr(fine_side_index)->vertex_average() + 100 * TOLERANCE * (fine_el->side_ptr(fine_side_index)->vertex_average() - fine_el->vertex_average());
        auto pl = mesh_copy->sub_point_locator();
        pl->enable_out_of_mesh_mode();
        auto const_neighbor = (*pl)(offset_point);
        pl->disable_out_of_mesh_mode();
        
        // We're at a boundary
        if (!const_neighbor)
        {
          std::cout << "Dismissing next candidate due to boundary at " << offset_point << std::endl;
          continue;
        }

        auto neighbor_fine_elem = mesh_copy->elem_ptr(const_neighbor->id());

        // Point locator finding a fine element inside
        if (fine_elements.find(neighbor_fine_elem) != fine_elements.end())
        {
          std::cout << "Dismissing next candidate due to being a fine element at " << offset_point << std::endl;
          continue;
        }

        // Note: these checks are not enough, because the starting mesh might not have elements
        // properly labeled with their refinement level
        // Case 1
        if (neighbor_fine_elem->level() < fine_el->level())
        {
          std::cout << "Dismissing from refinement level" << std::endl;
          continue;
        }
        // Case 3
        if (neighbor_fine_elem->level() > fine_el->level() &&
            coarse_elems.find(neighbor_fine_elem) == coarse_elems.end())
        {
          mooseInfo("Detected a refinement level transition while coarsening from the coarser side first. This case is not supported. Try to coarsen from the finer side instead. Coarsening will proceed without this element.");
          continue;
        }

        // Get the interior node for the next tentative coarse element
        // We can just use the index to get it from the next tentative fine element
        mooseAssert(neighbor_fine_elem->type() == QUAD4, "Only quad4 supported there for now");
        const auto opposite_node_index =
            (neighbor_fine_elem->get_node_index(coarse_node) + 2) % neighbor_fine_elem->n_nodes();
        auto neighbor_interior_node = neighbor_fine_elem->node_ptr(opposite_node_index);

        // avoid attempting to coarsen again an element we've already coarsened
        if (coarse_elems.find(neighbor_fine_elem) == coarse_elems.end())
        {
          // dont add candidates in blocks that have been coarsened enough
          for (const auto i : index_range(block_ids))
            if (block_ids[i] == neighbor_fine_elem->subdomain_id() &&
                coarsening[i] > coarse_step)
            {
              candidate_pairs.insert(std::make_pair(neighbor_fine_elem, neighbor_interior_node));
              break;
            }
        }
        std::cout << "Next candidate bank size: " << candidate_pairs.size() << std::endl;
      }
      // we just added an element. If we delete, we have to prepare for use first
      // preparing all the time will be expensive.
      // We definitely need to be adding the coarse elements to the mesh. Otherwise,
      // the coarsening of other elements wont detect the already coarsened elements
      // If we do not delete the fine elements, we will also be finding them for coarsening over
      // and over again
      mesh_copy->prepare_for_use();

      // Delete the elements used to build the coarse element
      bool parent_deleted = false;
      for (auto & fine_elem : fine_elements)
      {
        if (!fine_elem)
          continue;

        // If we are using libmesh h-refinement before coarsening (for testing), we cannot just leave
        // the parent in the mesh so we delete it too
        if (fine_elem->parent() && !parent_deleted)
        {
          parent_deleted = true;
          mesh_copy->delete_elem(fine_elem->parent());
        }

        mesh_copy->delete_elem(fine_elem);

        // Clean up the list of candidates from any deleted elements
        // candidate_pairs.erase(std::remove_if(candidate_pairs.begin(), candidate_pairs.end(),
        //                        [fine_elem](std::pair<Elem *, Node *> p){ return p.first == fine_elem; }),
        //                      candidate_pairs.end());
        // for (auto candidate : candidate_pairs)
        //   if (candidate.first == fine_elem)
        //     candidate_pairs.erase(candidate);
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

      //TODO down select which operations are necessary
      mesh_copy->contract();
      mesh_copy->prepare_for_use();
    }

    std::cout << "Should be 0: " << candidate_pairs.size() << std::endl;

    // We pick the configuration (eg starting node) for which we managed to coarsen the most
    // This isn't the best idea, as some coarsening could be invalid (non-conformalities)
    // Maybe we should examine for non-conformality here to make a decision?
    // It's expensive to do so in a global mesh-wide check though, maybe if we baked that check
    // into the coarsening work it would be more reasonable.
    if (_verbose)
      _console << "Step " << coarse_step + 1 << " attempt #" << start_node_index << " created " << coarse_elems.size() << " coarse elements." << std::endl;
    if (int(coarse_elems.size()) > max_num_coarsened)
    {
      mesh_return = std::move(mesh_copy);
      max_num_coarsened = coarse_elems.size();
    }
  }
  if (_verbose)
    _console << "Step " << coarse_step + 1 << " coarsened "
             << max_num_coarsened << " elements." << std::endl;
  coarse_step++;
  return recursive_coarsen(block_ids, mesh_return, coarsening, max, coarse_step);
}
