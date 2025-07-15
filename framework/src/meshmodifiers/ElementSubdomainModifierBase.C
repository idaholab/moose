//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainModifierBase.h"
#include "DisplacedProblem.h"
#include "MaterialWarehouse.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"
#include "libmesh/dof_map.h"
#include "libmesh/remote_elem.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/parameters.h"

InputParameters
ElementSubdomainModifierBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  // ESMs only operate on the undisplaced mesh to gather subdomain and sideset information. This
  // information is used to modify both the undisplaced and the displaced meshes. It is the
  // developer's responsibility to make sure the element IDs and sideset info are consistent across
  // both meshes.
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");

  params.addParam<std::vector<BoundaryName>>(
      "moving_boundaries",
      {},
      "Moving boundaries between subdomains. These boundaries (both sidesets and nodesets) will be "
      "updated for elements that change subdomain. The subdomains that each moving "
      "boundary lies between shall be specified using the parameter "
      "'moving_boundary_subdomain_pairs'. If one boundary and multiple subdomain pairs are "
      "specified, then it is assumed that the pairs all apply to the boundary. A boundary will be "
      "created on the mesh if it does not already exist.");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "moving_boundary_subdomain_pairs",
      {},
      "The subdomain pairs associated with each moving boundary. For each pair of subdomains, only "
      "the element side from the first subdomain will be added to the moving boundary, i.e., the "
      "side normal is pointing from the first subdomain to the second subdomain. The pairs shall "
      "be delimited by ';'. If a pair only has one subdomain, the moving boundary is associated "
      "with the subdomain's external boundary, i.e., when the elements have no neighboring "
      "elements.");

  params.addParam<std::vector<SubdomainName>>(
      "reinitialize_subdomains",
      {"ANY_BLOCK_ID"},
      "By default, any element which changes subdomain is reinitialized. If a list of subdomains "
      "(IDs or names) is provided, then only elements whose new subdomain is in the list will be "
      "reinitialized. If an empty list is set, then no elements will be reinitialized.");

  params.addParam<bool>(
      "old_subdomain_reinitialized",
      true,
      "This parameter must be set with a non-empty list in 'reinitialize_subdomains'. When set to "
      "the default true, the element's old subdomain is not considered when determining if an "
      "element should be reinitialized. If set to false, only elements whose old subdomain was not "
      "in 'reinitialize_subdomains' are reinitialized. ");

  params.addParam<std::vector<VariableName>>(
      "reinitialize_variables", {}, "Which variables to reinitialize when subdomain changes.");
  MooseEnum reinit_strategy("IC POLYNOMIAL_NEIGHBOR POLYNOMIAL_WHOLE POLYNOMIAL_NEARBY", "IC");
  params.addParam<std::vector<MooseEnum>>(
      "reinitialization_strategy",
      {reinit_strategy},
      "The strategy used to reinitialize the solution when elements change subdomain. If multiple "
      "strategies are provided, each strategy will be applied to the corresponding variable. If "
      "only one strategy is provided, it will be applied to all variables.");
  params.addParam<std::vector<UserObjectName>>(
      "polynomial_fitters",
      {},
      "List of NodalPatchRecovery UserObjects used to fit the polynomial for reinitialization. "
      "Only needed if 'reinitialization_strategy' is set to POLYNOMIAL_x.");
  params.addParam<int>(
      "nearby_kd_tree_leaf_max_size",
      10,
      "Maximum number of elements in a leaf node of the K-D tree used to search for nearby "
      "elements. Only needed if 'reinitialization_strategy' is set to POLYNOMIAL_NEARBY.");
  params.addParam<int>("nearby_element_threshold",
                       1,
                       "Threshold for considering elements as 'nearby' in the K-D tree search.");
  params.addParam<double>(
      "nearby_distance_threshold",
      -1.0,
      "Threshold for considering elements as 'nearby' in the K-D tree search. Only elements within "
      "this distance will be considered for polynomial fitting.");

  params.registerBase("MeshModifier");

  return params;
}

ElementSubdomainModifierBase::ElementSubdomainModifierBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _displaced_mesh(_displaced_problem ? &_displaced_problem->mesh() : nullptr),
    _nl_sys(_fe_problem.getNonlinearSystemBase(systemNumber())),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _old_subdomain_reinitialized(getParam<bool>("old_subdomain_reinitialized")),
    _npr_names(getParam<std::vector<UserObjectName>>("polynomial_fitters")),
    _reinit_vars(getParam<std::vector<VariableName>>("reinitialize_variables")),
    _nearby_element_threshold(getParam<int>("nearby_element_threshold")),
    _leaf_max_size(getParam<int>("nearby_kd_tree_leaf_max_size")),
    _nearby_distance_threshold(getParam<double>("nearby_distance_threshold"))
{
}

void
ElementSubdomainModifierBase::initialSetup()
{
  const std::vector<SubdomainName> subdomain_names_to_reinitialize =
      getParam<std::vector<SubdomainName>>("reinitialize_subdomains");

  if (std::find(subdomain_names_to_reinitialize.begin(),
                subdomain_names_to_reinitialize.end(),
                "ANY_BLOCK_ID") != subdomain_names_to_reinitialize.end())
    _subdomain_ids_to_reinitialize.push_back(Moose::ANY_BLOCK_ID);
  else
    _subdomain_ids_to_reinitialize = _mesh.getSubdomainIDs(subdomain_names_to_reinitialize);

  std::set<SubdomainID> set_subdomain_ids_to_reinitialize(_subdomain_ids_to_reinitialize.begin(),
                                                          _subdomain_ids_to_reinitialize.end());

  if (_old_subdomain_reinitialized == false &&
      (std::find(_subdomain_ids_to_reinitialize.begin(),
                 _subdomain_ids_to_reinitialize.end(),
                 Moose::ANY_BLOCK_ID) != _subdomain_ids_to_reinitialize.end() ||
       set_subdomain_ids_to_reinitialize == _mesh.meshSubdomains()))
    paramError("old_subdomain_reinitialized",
               "'old_subdomain_reinitialized' can only be set to false if "
               "reinitialize_subdomains does "
               "not cover the whole model, otherwise no elements will be reinitialized as it is "
               "impossible for an element's old subdomain to not be in the list.");
  else if (_old_subdomain_reinitialized == false && _subdomain_ids_to_reinitialize.empty())
    paramError("old_subdomain_reinitialized",
               "'old_subdomain_reinitialized' can only be set to false if "
               "reinitialize_subdomains is set to a non-empty list of subdomains, otherwise no "
               "elements will be reinitialized, as it is impossible for an element's new subdomain "
               "to be in the list.");

  auto bnd_names = getParam<std::vector<BoundaryName>>("moving_boundaries");
  auto bnd_ids = _mesh.getBoundaryIDs(bnd_names, true);
  const auto bnd_subdomains =
      getParam<std::vector<std::vector<SubdomainName>>>("moving_boundary_subdomain_pairs");

  if (bnd_names.size() == 1 && bnd_subdomains.size() > 1)
  {
    bnd_names.insert(bnd_names.end(), bnd_subdomains.size() - 1, bnd_names[0]);
    bnd_ids.insert(bnd_ids.end(), bnd_subdomains.size() - 1, bnd_ids[0]);
  }
  else if (bnd_names.size() != bnd_subdomains.size())
    paramError("moving_boundary_subdomain_pairs",
               "Each moving boundary must correspond to a pair of subdomains. ",
               bnd_names.size(),
               " boundaries are specified by the parameter 'moving_boundaries', while ",
               bnd_subdomains.size(),
               " subdomain pairs are provided. Alternatively, if one boundary and multiple "
               "subdomain pairs are provided, then the subdomain pairs all apply to one boundary.");

  for (auto i : index_range(bnd_names))
  {
    _moving_boundary_names[bnd_ids[i]] = bnd_names[i];

    if (bnd_subdomains[i].size() == 2)
      _moving_boundaries[{_mesh.getSubdomainID(bnd_subdomains[i][0]),
                          _mesh.getSubdomainID(bnd_subdomains[i][1])}] = bnd_ids[i];
    else if (bnd_subdomains[i].size() == 1)
      _moving_boundaries[{_mesh.getSubdomainID(bnd_subdomains[i][0]), Moose::INVALID_BLOCK_ID}] =
          bnd_ids[i];
    else
      paramError("moving_boundary_subdomain_pairs",
                 "Each subdomain pair must contain 1 or 2 subdomain names, but ",
                 bnd_subdomains[i].size(),
                 " are given.");
  }

  // Check if the variables to reinitialize exist in the system
  for (const auto & var_name : _reinit_vars)
    if (!_nl_sys.hasVariable(var_name) && !_aux_sys.hasVariable(var_name))
      paramError("reinitialize_variables", "Variable ", var_name, " does not exist.");

  // If no variables are specified, we default to all variables in the system
  if (!isParamSetByUser("reinitialize_variables"))
  {
    _reinit_vars = _nl_sys.getVariableNames();
    auto aux_vars = _aux_sys.getVariableNames();
    _reinit_vars.insert(_reinit_vars.end(), aux_vars.begin(), aux_vars.end());
  }

  // Determine the reinitialization strategy for each variable.
  //   (1) If they are of the same size, we perform a 1-to-1 mapping.
  //   (2) If only one strategy is provided, it applies to all variables.
  const auto reinit_strategy_in = getParam<std::vector<MooseEnum>>("reinitialization_strategy");
  if (reinit_strategy_in.size() == 1)
    _reinit_strategy.resize(_reinit_vars.size(), reinit_strategy_in[0].getEnum<ReinitStrategy>());
  else if (reinit_strategy_in.size() == _reinit_vars.size())
    for (const auto & e : reinit_strategy_in)
      _reinit_strategy.push_back(e.getEnum<ReinitStrategy>());
  else
    paramError(
        "reinitialization_strategy",
        "The 'reinitialization_strategy' parameter must have either a single value or a number "
        "of values equal to the number of 'reinitialize_variables'.");

  // Map variable names to the index of the nodal patch recovery user object
  _npr.resize(_npr_names.size());
  std::size_t npr_count = 0;
  for (auto i : index_range(_reinit_vars))
    if (_reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEIGHBOR ||
        _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_WHOLE ||
        _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEARBY)
    {
      _var_name_to_npr_idx[_reinit_vars[i]] = npr_count;
      if (npr_count >= _npr_names.size())
        paramError("polynomial_fitters",
                   "The number of polynomial fitters (",
                   _npr_names.size(),
                   ") is less than the number of variables to reinitialize with polynomial "
                   "extrapolation.");
      _npr[npr_count] = &getUserObjectByName<NodalPatchRecoveryBase>(_npr_names[npr_count]);
      npr_count++;
    }
  if (_npr_names.size() != npr_count)
    paramError("polynomial_fitters",
               "Mismatch between number of reinitialization strategies using polynomial "
               "extrapolation and polynomial fitters (expected: ",
               npr_count,
               ", given: ",
               _npr_names.size(),
               ").");
}

void
ElementSubdomainModifierBase::modify(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  // If nothing need to change, just return.
  // This will skip all mesh changes, and so no adaptivity mesh files will be written.
  auto n_moved_elem = moved_elems.size();
  gatherSum(n_moved_elem);
  if (n_moved_elem == 0)
    return;

  // Create moving boundaries on the undisplaced and displaced meshes
  //
  // Note: We do this _everytime_ because previous execution might have removed the sidesets and
  // nodesets. Most of the moving boundary algorithms below assume that the moving sidesets and
  // nodesets already exist on the mesh.
  createMovingBoundaries(_mesh);
  if (_displaced_mesh)
    createMovingBoundaries(*_displaced_mesh);

  // This has to be done *before* subdomain changes are applied
  findReinitializedElemsAndNodes(moved_elems);
  synchronizeReinitializedElems();

  // Apply cached subdomain changes
  applySubdomainChanges(moved_elems, _mesh);
  if (_displaced_mesh)
    applySubdomainChanges(moved_elems, *_displaced_mesh);

  // Update moving boundaries
  gatherMovingBoundaryChanges(moved_elems);
  applyMovingBoundaryChanges(_mesh);
  if (_displaced_mesh)
    applyMovingBoundaryChanges(*_displaced_mesh);

  _solved_elem_ids_for_npr.clear();
  if (!_npr.empty())
    for (auto i : index_range(_reinit_vars))
    {
      const auto var_name = _reinit_vars[i];
      if (_var_name_to_npr_idx.find(var_name) == _var_name_to_npr_idx.end())
        continue;

      const int npr_idx = _var_name_to_npr_idx[var_name];

      auto & reinit_strategy = _reinit_strategy[i];

      auto it = _solved_elem_ids_for_npr.find(reinit_strategy);
      // do not find the strategy, so we need to gather neighbor elements
      if (it == _solved_elem_ids_for_npr.end())
        gatherNeighborElementsForActivatedNodes(reinit_strategy);

      _npr[npr_idx]->cacheAdditionalElements(_solved_elem_ids_for_npr[reinit_strategy],
                                             true /*synchronizeAebe*/);
    }

  // Reinit equation systems
  _fe_problem.meshChanged(
      /*intermediate_change=*/false, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);

  // Clear the serialized solution after the mesh has changed
  _sys.cleanSerializedSolution();

  // Initialize solution and stateful material properties
  applyIC(/*displaced=*/false);
  if (_fe_problem.getMaterialWarehouse().hasActiveObjects(0))
    initElementStatefulProps(/*displaced=*/false);

  if (_displaced_mesh)
  {
    applyIC(/*displaced=*/true);
    if (_fe_problem.getMaterialWarehouse().hasActiveObjects(0))
      initElementStatefulProps(/*displaced=*/true);
  }
}

void
ElementSubdomainModifierBase::createMovingBoundaries(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();
  for (const auto & [bnd_id, bnd_name] : _moving_boundary_names)
  {
    bnd_info.sideset_name(bnd_id) = bnd_name;
    bnd_info.nodeset_name(bnd_id) = bnd_name;
  }
}

void
ElementSubdomainModifierBase::applySubdomainChanges(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems,
    MooseMesh & mesh)
{
  for (const auto & [elem_id, subdomain] : moved_elems)
  {
    // Change the element's subdomain ID
    auto elem = mesh.elemPtr(elem_id);
    const auto & [from, to] = subdomain;
    mooseAssert(elem->subdomain_id() == from, "Inconsistent element subdomain ID.");
    elem->subdomain_id() = to;

    // Change the ancestors' (if any) subdomain ID
    setAncestorsSubdomainIDs(elem, to);
  }

  // Synchronize ghost element subdomain changes
  libMesh::SyncSubdomainIds sync(mesh.getMesh());
  Parallel::sync_dofobject_data_by_id(
      mesh.getMesh().comm(), mesh.getMesh().elements_begin(), mesh.getMesh().elements_end(), sync);
}

void
ElementSubdomainModifierBase::setAncestorsSubdomainIDs(Elem * elem, const SubdomainID subdomain_id)
{
  auto curr_elem = elem;

  for (unsigned int i = curr_elem->level(); i > 0; --i)
  {
    // Change the parent's subdomain
    curr_elem = curr_elem->parent();
    curr_elem->subdomain_id() = subdomain_id;
  }
}

void
ElementSubdomainModifierBase::gatherMovingBoundaryChanges(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  // Clear moving boundary changes from last execution
  _add_element_sides.clear();
  _add_neighbor_sides.clear();
  _remove_element_sides.clear();
  _remove_neighbor_sides.clear();

  const auto & sidesets = _mesh.getMesh().get_boundary_info().get_sideset_map();

  for (const auto & [elem_id, subdomain_assignment] : moved_elems)
  {
    auto elem = _mesh.elemPtr(elem_id);

    // The existing moving boundaries on the element side should be removed
    for (auto itr = sidesets.lower_bound(elem); itr != sidesets.upper_bound(elem); itr++)
      if (_moving_boundary_names.count(itr->second.second))
        _remove_element_sides[elem->id()].emplace(itr->second.first, itr->second.second);

    for (auto side : elem->side_index_range())
    {
      auto neigh = elem->neighbor_ptr(side);

      // Don't mess with remote element neighbor
      if (neigh && neigh == libMesh::remote_elem)
        continue;
      // If neighbor doesn't exist
      else if (!neigh)
        gatherMovingBoundaryChangesHelper(elem, side, nullptr, 0);
      // If neighbor exists
      else
      {
        auto neigh_side = neigh->which_neighbor_am_i(elem);

        if (neigh->active())
          gatherMovingBoundaryChangesHelper(elem, side, neigh, neigh_side);
        else
        {
          // Find the active neighbors of the element
          std::vector<const Elem *> active_neighs;
          // Neighbor has active children, they are neighbors of the element along that side
          mooseAssert(!neigh->subactive(),
                      "The case where the active neighbor is an ancestor of this neighbor is not "
                      "handled at this time.");
          neigh->active_family_tree_by_neighbor(active_neighs, elem);

          for (auto active_neigh : active_neighs)
            gatherMovingBoundaryChangesHelper(elem, side, active_neigh, neigh_side);
        }
      }
    }
  }
}

void
ElementSubdomainModifierBase::gatherMovingBoundaryChangesHelper(const Elem * elem,
                                                                unsigned short side,
                                                                const Elem * neigh,
                                                                unsigned short neigh_side)
{
  const auto & sidesets = _mesh.getMesh().get_boundary_info().get_sideset_map();

  // Detect element side change
  SubdomainPair subdomain_pair = {elem->subdomain_id(),
                                  neigh ? neigh->subdomain_id() : Moose::INVALID_BLOCK_ID};
  if (_moving_boundaries.count(subdomain_pair))
    _add_element_sides[elem->id()].emplace(side, _moving_boundaries.at(subdomain_pair));

  if (neigh)
  {
    // The existing moving boundaries on the neighbor side should be removed
    for (auto itr = sidesets.lower_bound(neigh); itr != sidesets.upper_bound(neigh); itr++)
      if (itr->second.first == neigh_side && _moving_boundary_names.count(itr->second.second))
        _remove_neighbor_sides[neigh->id()].emplace(itr->second.first, itr->second.second);

    // Detect neighbor side change (by reversing the subdomain pair)
    subdomain_pair = {subdomain_pair.second, subdomain_pair.first};
    if (_moving_boundaries.count(subdomain_pair))
      _add_neighbor_sides[neigh->id()].emplace(neigh_side, _moving_boundaries.at(subdomain_pair));
  }
}

void
ElementSubdomainModifierBase::applyMovingBoundaryChanges(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();

  // Remove all boundary nodes from the previous moving boundaries
  auto nodesets = bnd_info.get_nodeset_map();
  for (const auto & [node_id, bnd] : nodesets)
    if (_moving_boundary_names.count(bnd))
      bnd_info.remove_node(node_id, bnd);

  // Keep track of ghost element changes
  std::unordered_map<processor_id_type,
                     std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>>>
      add_ghost_sides, remove_ghost_sides;

  // Remove element sides from moving boundaries
  for (const auto & [elem_id, sides] : _remove_element_sides)
    for (const auto & [side, bnd] : sides)
      bnd_info.remove_side(mesh.elemPtr(elem_id), side, bnd);

  // Remove neighbor sides from moving boundaries
  for (const auto & [elem_id, sides] : _remove_neighbor_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
    {
      bnd_info.remove_side(elem, side, bnd);
      // Keep track of changes to ghosted elements
      if (elem->processor_id() != processor_id())
        remove_ghost_sides[elem->processor_id()].push_back({elem_id, side, bnd});
    }
  }

  // Add element sides to moving boundaries
  for (const auto & [elem_id, sides] : _add_element_sides)
    for (const auto & [side, bnd] : sides)
      bnd_info.add_side(mesh.elemPtr(elem_id), side, bnd);

  // Add neighbor sides to moving boundaries
  for (const auto & [elem_id, sides] : _add_neighbor_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
    {
      bnd_info.add_side(elem, side, bnd);
      // Keep track of changes to ghosted elements
      if (elem->processor_id() != processor_id())
        add_ghost_sides[elem->processor_id()].push_back({elem_id, side, bnd});
    }
  }

  Parallel::push_parallel_vector_data(
      bnd_info.comm(),
      add_ghost_sides,
      [&mesh,
       &bnd_info](processor_id_type,
                  const std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>> & received)
      {
        for (const auto & [elem_id, side, bnd] : received)
          bnd_info.add_side(mesh.elemPtr(elem_id), side, bnd);
      });

  Parallel::push_parallel_vector_data(
      bnd_info.comm(),
      remove_ghost_sides,
      [&mesh,
       &bnd_info](processor_id_type,
                  const std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>> & received)
      {
        for (const auto & [elem_id, side, bnd] : received)
          bnd_info.remove_side(mesh.elemPtr(elem_id), side, bnd);
      });

  bnd_info.parallel_sync_side_ids();
  bnd_info.parallel_sync_node_ids();
  mesh.update();
}

void
ElementSubdomainModifierBase::meshChanged()
{
  // Clear cached ranges
  _reinitialized_elem_range.reset();
  _reinitialized_displaced_elem_range.reset();
  _reinitialized_bnd_node_range.reset();
  _reinitialized_displaced_bnd_node_range.reset();
  _reinitialized_node_range.reset();
  _reinitialized_node_range_from_bnd_nodes.reset();
  _reinitialized_displaced_node_range_from_bnd_nodes.reset();

  updateAMRMovingBoundary(_mesh);
  if (_displaced_mesh)
    updateAMRMovingBoundary(*_displaced_mesh);
}

void
ElementSubdomainModifierBase::updateAMRMovingBoundary(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();
  auto sidesets = bnd_info.get_sideset_map();
  for (const auto & i : sidesets)
  {
    auto elem = i.first;
    auto side = i.second.first;
    auto bnd = i.second.second;
    if (_moving_boundary_names.count(bnd) && !elem->active())
    {
      bnd_info.remove_side(elem, side, bnd);

      std::vector<const Elem *> elem_family;
      elem->active_family_tree_by_side(elem_family, side);
      for (auto felem : elem_family)
        bnd_info.add_side(felem, side, bnd);
    }
  }

  bnd_info.parallel_sync_side_ids();
  bnd_info.parallel_sync_node_ids();
}

void
ElementSubdomainModifierBase::findReinitializedElemsAndNodes(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  // Clear cached element reinitialization data
  _reinitialized_elems.clear();
  _reinitialized_nodes.clear();
  _non_reinit_nodes_on_reinit_elems.clear();

  std::unordered_set<dof_id_type> local_reinitialized_nodes, non_reinit_nodes_on_reinit_elems;

  for (const auto & [elem_id, subdomain] : moved_elems)
  {
    mooseAssert(_mesh.elemPtr(elem_id)->active(), "Moved elements should be active");
    // Default: any element that changes subdomain is reinitialized
    if (std::find(_subdomain_ids_to_reinitialize.begin(),
                  _subdomain_ids_to_reinitialize.end(),
                  Moose::ANY_BLOCK_ID) != _subdomain_ids_to_reinitialize.end())
      _reinitialized_elems.insert(elem_id);
    else // Reinitialize if new subdomain is in list of subdomains to be reinitialized
    {
      const auto & [from, to] = subdomain;
      if (subdomainIsReinitialized(to) && _old_subdomain_reinitialized)
        _reinitialized_elems.insert(elem_id);
      // Only reinitialize if original subdomain is not in list of subdomains
      else if (subdomainIsReinitialized(to) && !_old_subdomain_reinitialized &&
               !subdomainIsReinitialized(from))
        _reinitialized_elems.insert(elem_id);
      else // New subdomain is not in list of subdomains
        continue;
    }

    const auto elem = _mesh.elemPtr(elem_id);
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
    {
      if (nodeIsNewlyReinitialized(elem->node_id(i)))
        local_reinitialized_nodes.insert(elem->node_id(i));
      else
        non_reinit_nodes_on_reinit_elems.insert(elem->node_id(i));
    }
  }

  // Convert to vector and allgather across processors
  std::vector<dof_id_type> local(local_reinitialized_nodes.begin(),
                                 local_reinitialized_nodes.end());
  std::vector<dof_id_type> non_reinit_nodes(non_reinit_nodes_on_reinit_elems.begin(),
                                            non_reinit_nodes_on_reinit_elems.end());

  std::vector<std::vector<dof_id_type>> gathered;
  _mesh.comm().allgather(local, gathered);
  std::vector<std::vector<dof_id_type>> non_reinit_nodes_gathered;
  _mesh.comm().allgather(non_reinit_nodes, non_reinit_nodes_gathered);

  // Collect globally unique activated nodes
  std::unordered_set<dof_id_type> unique_nodes;
  for (const auto & vec : gathered)
    unique_nodes.insert(vec.begin(), vec.end());

  // Collect globally unique non-reinitialized nodes
  std::unordered_set<dof_id_type> unique_non_reinit_nodes;
  for (const auto & vec : non_reinit_nodes_gathered)
    unique_non_reinit_nodes.insert(vec.begin(), vec.end());

  // Filters the globally reinitialized nodes and stores only those owned by this processor.
  for (auto id : unique_nodes)
  {
    const auto node = _mesh.nodePtr(id);
    if (node->processor_id() == _mesh.processor_id())
      _reinitialized_nodes.insert(id);
  }

  // Filters the globally non-reinitialized nodes and stores only those owned by this processor.
  for (auto id : unique_non_reinit_nodes)
  {
    const auto node = _mesh.nodePtr(id);
    if (node->processor_id() == _mesh.processor_id())
      _non_reinit_nodes_on_reinit_elems.insert(id);
  }
}

bool
ElementSubdomainModifierBase::subdomainIsReinitialized(SubdomainID id) const
{
  // Default: any element that changes subdomain is reinitialized
  if (std::find(_subdomain_ids_to_reinitialize.begin(),
                _subdomain_ids_to_reinitialize.end(),
                Moose::ANY_BLOCK_ID) != _subdomain_ids_to_reinitialize.end())
    return true;

  // Is subdomain in list of subdomains to be reinitialized
  return std::find(_subdomain_ids_to_reinitialize.begin(),
                   _subdomain_ids_to_reinitialize.end(),
                   id) != _subdomain_ids_to_reinitialize.end();
}

bool
ElementSubdomainModifierBase::nodeIsNewlyReinitialized(dof_id_type node_id) const
{
  // If any of the node neighbor elements has reinitialized, then the node is NOT newly
  // reinitialized.
  for (auto neighbor_elem_id : _mesh.nodeToElemMap().at(node_id))
    if (subdomainIsReinitialized(_mesh.elemPtr(neighbor_elem_id)->subdomain_id()))
      return false;
  return true;
}

void
ElementSubdomainModifierBase::applyIC(bool displaced)
{
  // Set of variable names that are not part of the _reinit_vars
  std::set<VariableName> ic_target_vars_names_except_ic_vars;
  std::set<VariableName> all_vars_names;
  auto collectVarsAndMarkNonICVars = [&](SystemBase & sys) -> void
  {
    const auto & vars = sys.getVariables(_tid);
    for (const auto & ivar : vars)
    {
      all_vars_names.insert(ivar->name());
      if (std::find(_reinit_vars.begin(), _reinit_vars.end(), ivar->name()) == _reinit_vars.end())
        ic_target_vars_names_except_ic_vars.insert(ivar->name());
    }
  };

  collectVarsAndMarkNonICVars(_nl_sys);
  collectVarsAndMarkNonICVars(_aux_sys);

  // store the values of the non-reinitialized nodes on the reinitialized elements
  storeValuesFromNonReinitNodes(all_vars_names);

  // note: from IC -> current
  _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                   reinitializedBndNodeRange(displaced),
                                                   ic_target_vars_names_except_ic_vars);

  // Loop over each variable and initialize
  for (auto i : index_range(_reinit_vars))
  {
    if (_reinit_strategy[i] == ReinitStrategy::IC)
      // note: from IC -> current
      _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                       reinitializedBndNodeRange(displaced),
                                                       {{_reinit_vars[i]}});
    else if (_reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEIGHBOR ||
             _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_WHOLE ||
             _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEARBY)
    {
      projectNprIC(_reinit_vars[i], displaced, _reinit_strategy[i]);
    }
    else
      mooseError("Unknown initial condition strategy");
  }

  // Set back the non-reinitialized nodes to its original values
  restoreValuesToNonReinitNodes(all_vars_names);

  mooseAssert(_fe_problem.numSolverSystems() < 2,
              "This code was written for a single nonlinear system");
  // Set old and older solutions on the reinitialized dofs to the reinitialized values
  // note: from current -> old -> older
  setOldAndOlderSolutions(_fe_problem.getNonlinearSystemBase(_sys.number()),
                          reinitializedElemRange(displaced),
                          reinitializedBndNodeRange(displaced));
  setOldAndOlderSolutions(_fe_problem.getAuxiliarySystem(),
                          reinitializedElemRange(displaced),
                          reinitializedBndNodeRange(displaced));

  // Note: Need method to handle solve failures at timesteps where subdomain changes. The old
  // solutions are now set to the reinitialized values. Does this impact restoring solutions
}

void
ElementSubdomainModifierBase::storeValuesFromNonReinitNodes(
    const std::set<VariableName> & vars_names)
{
  _var_to_dofs_values_from_nonreinit_nodes.clear();

  for (const auto & var_name : vars_names)
  {
    const auto & var = _fe_problem.getStandardVariable(0, var_name);
    const auto var_num = var.number();

    SystemBase & sys = _aux_sys.hasVariable(var_name)
                           ? static_cast<SystemBase &>(_fe_problem.getAuxiliarySystem())
                           : static_cast<SystemBase &>(_fe_problem.getNonlinearSystemBase(
                                 _fe_problem.systemNumForVariable(var_name)));

    const auto & current_solution = *sys.system().current_local_solution;
    DofMap & dof_map = sys.dofMap();

    std::set<dof_id_type> dof_ids;
    for (const auto & node_id : _non_reinit_nodes_on_reinit_elems)
    {
      const auto & node = _mesh.nodePtr(node_id);
      std::vector<dof_id_type> dof_indices;
      dof_map.dof_indices(node, dof_indices, var_num);
      dof_ids.insert(dof_indices.begin(), dof_indices.end());
    }

    std::vector<Number> values;
    for (auto dof : dof_ids)
      values.push_back(current_solution(dof));

    _var_to_dofs_values_from_nonreinit_nodes[var_name] = {
        std::vector<dof_id_type>(dof_ids.begin(), dof_ids.end()), values};
  }
}

void
ElementSubdomainModifierBase::restoreValuesToNonReinitNodes(
    const std::set<VariableName> & vars_names)
{
  for (const auto & var_name : vars_names)
  {
    SystemBase & sys = _aux_sys.hasVariable(var_name)
                           ? static_cast<SystemBase &>(_fe_problem.getAuxiliarySystem())
                           : static_cast<SystemBase &>(_fe_problem.getNonlinearSystemBase(
                                 _fe_problem.systemNumForVariable(var_name)));

    const auto & dof_ids = _var_to_dofs_values_from_nonreinit_nodes[var_name].first;
    const auto & values = _var_to_dofs_values_from_nonreinit_nodes[var_name].second;

    for (const int i : index_range(dof_ids))
      sys.solution().set(dof_ids[i], values[i]);

    sys.solution().close();
    sys.solution().localize(*sys.system().current_local_solution, sys.dofMap().get_send_list());
  }
}

void
ElementSubdomainModifierBase::initElementStatefulProps(bool displaced)
{
  _fe_problem.initElementStatefulProps(reinitializedElemRange(displaced), /*threaded=*/true);
}

ConstElemRange &
ElementSubdomainModifierBase::reinitializedElemRange(bool displaced)
{
  if (!displaced && _reinitialized_elem_range)
    return *_reinitialized_elem_range.get();

  if (displaced && _reinitialized_displaced_elem_range)
    return *_reinitialized_displaced_elem_range.get();

  // Create a vector of the newly reinitialized elements
  std::vector<Elem *> elems;
  for (auto elem_id : _reinitialized_elems)
  {
    elems.push_back(displaced ? _displaced_mesh->elemPtr(elem_id) : _mesh.elemPtr(elem_id));
  }

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  if (!displaced)
    _reinitialized_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);
  else
    _reinitialized_displaced_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);

  return reinitializedElemRange(displaced);
}

ConstBndNodeRange &
ElementSubdomainModifierBase::reinitializedBndNodeRange(bool displaced)
{
  if (!displaced && _reinitialized_bnd_node_range)
    return *_reinitialized_bnd_node_range.get();

  if (displaced && _reinitialized_displaced_bnd_node_range)
    return *_reinitialized_displaced_bnd_node_range.get();

  // Create a vector of the newly reinitialized boundary nodes
  std::vector<const BndNode *> nodes;
  auto bnd_nodes =
      displaced ? _displaced_mesh->getBoundaryNodeRange() : _mesh.getBoundaryNodeRange();
  for (auto bnd_node : *bnd_nodes)
    if (bnd_node->_node)
      if (_reinitialized_nodes.count(bnd_node->_node->id()))
      {
        nodes.push_back(bnd_node);
      }

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  if (!displaced)
    _reinitialized_bnd_node_range =
        std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);
  else
    _reinitialized_displaced_bnd_node_range =
        std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);

  return reinitializedBndNodeRange(displaced);
}

ConstNodeRange &
ElementSubdomainModifierBase::reinitializedNodeRange()
{
  if (_reinitialized_node_range)
    return *_reinitialized_node_range.get();

  // Create a vector of the newly reinitialized nodes
  std::vector<const Node *> nodes;

  for (auto node_id : _reinitialized_nodes)
  {
    nodes.push_back(_mesh.nodePtr(node_id)); // displaced mesh shares the same node object
  }

  // Make some fake node iterators defining this vector of nodes
  Node * const * node_itr_begin = const_cast<Node * const *>(nodes.data());
  Node * const * node_itr_end = node_itr_begin + nodes.size();

  const auto nodes_begin = MeshBase::const_node_iterator(
      node_itr_begin, node_itr_end, Predicates::NotNull<const Node * const *>());
  const auto nodes_end = MeshBase::const_node_iterator(
      node_itr_end, node_itr_end, Predicates::NotNull<const Node * const *>());

  _reinitialized_node_range = std::make_unique<ConstNodeRange>(nodes_begin, nodes_end);

  return *_reinitialized_node_range.get();
}

ConstNodeRange &
ElementSubdomainModifierBase::reinitializedNodeRangeFromBndNodes(bool displaced)
{
  if (!displaced && _reinitialized_node_range_from_bnd_nodes)
    return *_reinitialized_node_range_from_bnd_nodes.get();

  if (displaced && _reinitialized_displaced_node_range_from_bnd_nodes)
    return *_reinitialized_displaced_node_range_from_bnd_nodes.get();

  std::vector<const Node *> nodes;

  auto bnd_nodes =
      displaced ? _displaced_mesh->getBoundaryNodeRange() : _mesh.getBoundaryNodeRange();

  for (auto bnd_node : *bnd_nodes)
    if (bnd_node->_node && _reinitialized_nodes.count(bnd_node->_node->id()))
      nodes.push_back(bnd_node->_node);

  // Fake iterators
  Node * const * node_itr_begin = const_cast<Node * const *>(nodes.data());
  Node * const * node_itr_end = node_itr_begin + nodes.size();

  const auto nodes_begin = MeshBase::const_node_iterator(
      node_itr_begin, node_itr_end, Predicates::NotNull<const Node * const *>());
  const auto nodes_end = MeshBase::const_node_iterator(
      node_itr_end, node_itr_end, Predicates::NotNull<const Node * const *>());

  if (!displaced)
    _reinitialized_node_range_from_bnd_nodes =
        std::make_unique<ConstNodeRange>(nodes_begin, nodes_end);
  else
    _reinitialized_displaced_node_range_from_bnd_nodes =
        std::make_unique<ConstNodeRange>(nodes_begin, nodes_end);

  return reinitializedNodeRangeFromBndNodes(displaced);
}

void
ElementSubdomainModifierBase::setOldAndOlderSolutions(SystemBase & sys,
                                                      ConstElemRange & elem_range,
                                                      ConstBndNodeRange & bnd_node_range)
{

  for (auto bnd : bnd_node_range)
  {
    const Node * bnode = bnd->_node;
    if (!bnode)
      continue;
  }

  // Don't do anything if this is a steady simulation
  if (!sys.hasSolutionState(1))
    return;

  NumericVector<Number> & current_solution = *sys.system().current_local_solution;
  NumericVector<Number> & old_solution = sys.solutionOld();
  NumericVector<Number> * older_solution = sys.hasSolutionState(2) ? &sys.solutionOlder() : nullptr;

  // Get dofs for the reinitialized elements and nodes
  DofMap & dof_map = sys.dofMap();
  std::vector<dof_id_type> dofs;

  for (auto & elem : elem_range)
  {
    std::vector<dof_id_type> elem_dofs;
    dof_map.dof_indices(elem, elem_dofs);
    dofs.insert(dofs.end(), elem_dofs.begin(), elem_dofs.end());
  }

  for (auto & bnd_node : bnd_node_range)
  {
    std::vector<dof_id_type> bnd_node_dofs;
    dof_map.dof_indices(bnd_node->_node, bnd_node_dofs);
    dofs.insert(dofs.end(), bnd_node_dofs.begin(), bnd_node_dofs.end());
  }

  // Set the old and older solutions to match the reinitialization
  for (auto dof : dofs)
  {
    old_solution.set(dof, current_solution(dof));
    if (older_solution)
      older_solution->set(dof, current_solution(dof));
  }

  old_solution.close();
  if (older_solution)
    older_solution->close();
}

void
ElementSubdomainModifierBase::gatherNeighborElementsForActivatedNodes(
    ReinitStrategy & reinit_strategy)
{
  if (!_solved_elem_ids_for_npr[reinit_strategy].empty())
    return;

  mooseAssert(reinit_strategy == ReinitStrategy::POLYNOMIAL_WHOLE ||
                  reinit_strategy == ReinitStrategy::POLYNOMIAL ||
                  reinit_strategy == ReinitStrategy::POLYNOMIAL_NEARBY,
              "reinit strategy must be POLYNOMIAL_WHOLE, POLYNOMIAL, or POLYNOMIAL_NEARBY");

  auto localToGlobal =
      [&](const auto & local_ids,
          std::vector<typename std::decay<decltype(local_ids[0])>::type> & global_ids,
          bool sort_and_remove_duplicates = false) -> void
  {
    using IDType = typename std::decay<decltype(local_ids[0])>::type;

    std::vector<std::vector<IDType>> gathered;
    _mesh.comm().allgather(local_ids, gathered);

    global_ids.clear();
    for (const auto & vec : gathered)
      global_ids.insert(global_ids.end(), vec.begin(), vec.end());

    if (sort_and_remove_duplicates)
    { // remove duplicates cause issue for std::vector<Point>
      std::sort(global_ids.begin(), global_ids.end());
      global_ids.erase(std::unique(global_ids.begin(), global_ids.end()), global_ids.end());
    }
  };

  auto localToGlobalPair =
      [&](const auto & local_vals1,
          const auto & local_vals2,
          std::vector<typename std::decay<decltype(local_vals1[0])>::type> & global_vals1,
          std::vector<typename std::decay<decltype(local_vals2[0])>::type> & global_vals2) -> void
  {
    mooseAssert(local_vals1.size() == local_vals2.size(),
                "local_vals1 and local_vals2 must have the same size!");

    using Type1 = typename std::decay<decltype(local_vals1[0])>::type;
    using Type2 = typename std::decay<decltype(local_vals2[0])>::type;

    std::vector<std::vector<Type1>> gathered1;
    std::vector<std::vector<Type2>> gathered2;

    _mesh.comm().allgather(local_vals1, gathered1);
    _mesh.comm().allgather(local_vals2, gathered2);

    global_vals1.clear();
    global_vals2.clear();
    for (std::size_t i = 0; i < gathered1.size(); ++i)
    {
      global_vals1.insert(global_vals1.end(), gathered1[i].begin(), gathered1[i].end());
      global_vals2.insert(global_vals2.end(), gathered2[i].begin(), gathered2[i].end());
    }
  };

  // 0.  Pre-checks and caching
  if (_global_reinitialized_elems.empty())
    return;

  // (a) For O(1) lookup; use a set for elements and a set for nodes
  std::unordered_set<dof_id_type> reinit_elem_set(_global_reinitialized_elems.begin(),
                                                  _global_reinitialized_elems.end());
  std::unordered_set<dof_id_type> reinit_node_set;

  // (b) Collect all nodes owned by reinitialized elements
  for (const auto & eid : _global_reinitialized_elems)
  {
    const Elem * e = _mesh.elemPtr(eid);
    if (!e)
      continue; // Not necessarily local; skip if nullptr

    for (unsigned n = 0; n < e->n_nodes(); ++n)
      reinit_node_set.insert(e->node_id(n)); // already prevented duplicates
  }

  std::unordered_set<dof_id_type> patch_elem_set; // Prevent duplicates

  if (reinit_strategy == ReinitStrategy::POLYNOMIAL_NEARBY)
  {
    std::vector<Point> local_centroids;
    std::vector<dof_id_type> local_elem_ids;
    for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
    {
      // Skip if this is a reinitialized element
      if (reinit_elem_set.count(elem->id()))
        continue;

      // Skip if element is unsolved (inactive in computational domain)
      if (std::find(_subdomain_ids_to_reinitialize.begin(),
                    _subdomain_ids_to_reinitialize.end(),
                    elem->subdomain_id()) == _subdomain_ids_to_reinitialize.end())
        continue;

      BoundingBox bbox = elem->loose_bounding_box();
      const Point box_vec = bbox.second - bbox.first;
      _min_diag_length = std::min(_min_diag_length, box_vec.norm());

      local_centroids.push_back(elem->vertex_average());
      local_elem_ids.push_back(elem->id());
    }

    localToGlobalPair(
        local_centroids, local_elem_ids, _centroids_of_elements, _kd_tree_sequence_elem_id_map);

    _kd_tree = new KDTree(_centroids_of_elements, _leaf_max_size);

    _mesh.comm().min(_min_diag_length); // TIMPI

    if (_nearby_distance_threshold < 0.0)
      _nearby_distance_threshold = _nearby_element_threshold * _min_diag_length;

    for (const auto & elem_id : _global_reinitialized_elems)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      if (!elem)
        continue;

      const Point & centroid = elem->vertex_average();

      std::vector<nanoflann::ResultItem<std::size_t, Real>> matches;

      _kd_tree->radiusSearch(centroid, _nearby_distance_threshold, matches);

      for (const auto & m : matches)
      {
        dof_id_type neighbor_elem_id =
            _kd_tree_sequence_elem_id_map[static_cast<unsigned int>(m.first)];

        // Skip if already added to patch
        if (patch_elem_set.count(neighbor_elem_id))
          continue;

        _solved_elem_ids_for_npr[reinit_strategy].push_back(neighbor_elem_id);
        patch_elem_set.insert(neighbor_elem_id);
      }
    }
  }
  else
  {
    // Pass : Traverse local active elements to find neighbors sharing nodes with reinit elements
    for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
    {
      const dof_id_type eid = elem->id();

      // (a) Skip if this is a reinitialized element
      if (reinit_elem_set.count(eid))
        continue;

      // (b) Skip if already added to patch
      if (patch_elem_set.count(eid))
        continue;

      // (c) Skip if element is unsolved (inactive in computational domain)
      if (std::find(_subdomain_ids_to_reinitialize.begin(),
                    _subdomain_ids_to_reinitialize.end(),
                    elem->subdomain_id()) == _subdomain_ids_to_reinitialize.end())
        continue;

      if (reinit_strategy == ReinitStrategy::POLYNOMIAL_NEIGHBOR)
      {
        // (d) Check if any node is in reinit_node_set
        bool share_with_reinit = false;
        for (unsigned n = 0; n < elem->n_nodes(); ++n)
          if (reinit_node_set.count(elem->node_id(n)))
          {
            share_with_reinit = true;
            break;
          }

        // (e) If shares node with reinit element, add to patch
        if (share_with_reinit)
        {
          patch_elem_set.insert(eid);
          _solved_elem_ids_for_npr[reinit_strategy].push_back(eid);
        }
      }
      else if (reinit_strategy == ReinitStrategy::POLYNOMIAL_WHOLE)
      {
        patch_elem_set.insert(eid);
        _solved_elem_ids_for_npr[reinit_strategy].push_back(eid);
      }
    }
  }

  localToGlobal(_solved_elem_ids_for_npr[reinit_strategy],
                _solved_elem_ids_for_npr[reinit_strategy],
                true /*sort_and_remove_duplicates*/);
}

void
ElementSubdomainModifierBase::synchronizeReinitializedElems()
{
  std::vector<dof_id_type> local(_reinitialized_elems.begin(), _reinitialized_elems.end());
  std::vector<std::vector<dof_id_type>> gathered;
  _mesh.comm().allgather(local, gathered);

  _global_reinitialized_elems.clear();
  for (const auto & vec : gathered)
    _global_reinitialized_elems.insert(_global_reinitialized_elems.end(), vec.begin(), vec.end());
}

void
ElementSubdomainModifierBase::projectNprIC(const VariableName & var_name,
                                           bool displaced,
                                           ReinitStrategy reinit_strategy)
{
  const auto & coef = _npr[_var_name_to_npr_idx[var_name]]->getCoefficients(
      _solved_elem_ids_for_npr[reinit_strategy]);

  const unsigned dim = _mesh.dimension();

  libMesh::Parameters function_parameters;

  const auto & multi_index = _npr[_var_name_to_npr_idx[var_name]]->multiIndex();

  function_parameters.set<std::vector<std::vector<unsigned int>>>("multi_index") = multi_index;

  std::vector<Real> coef_vec(coef.size());
  for (auto i = 0; i < coef.size(); ++i)
    coef_vec[i] = coef(i);

  function_parameters.set<std::vector<Real>>("multi_index_coefficients") = coef_vec;
  function_parameters.set<unsigned int>("dimension_for_projection") = dim;

  // Define projection function
  auto poly_func = [](const Point & p,
                      const libMesh::Parameters & parameters,
                      const std::string &,
                      const std::string &) -> libMesh::Number
  {
    const auto & multi_index =
        parameters.get<std::vector<std::vector<unsigned int>>>("multi_index");
    const auto & coeffs = parameters.get<std::vector<Real>>("multi_index_coefficients");

    Real val = 0.0;

    for (unsigned int r = 0; r < multi_index.size(); r++)
    {
      Real monomial = 1.0;
      for (unsigned int c = 0; c < multi_index[r].size(); c++)
      {
        const auto power = multi_index[r][c];
        if (power == 0)
          continue;

        monomial *= std::pow(p(c), power);
      }
      val += coeffs[r] * monomial;
    }

    return val;
  };

  // Define gradient
  auto poly_func_grad = [](const Point & p,
                           const libMesh::Parameters & parameters,
                           const std::string &,
                           const std::string &) -> libMesh::Gradient
  {
    const unsigned int dim = parameters.get<unsigned int>("dimension_for_projection");

    const auto & multi_index =
        parameters.get<std::vector<std::vector<unsigned int>>>("multi_index");
    const auto & coeffs = parameters.get<std::vector<Real>>("multi_index_coefficients");

    libMesh::Gradient grad; // Zero-initialized

    for (unsigned int r = 0; r < multi_index.size(); ++r)
    {
      const auto & powers = multi_index[r];
      const Real coef = coeffs[r];

      for (unsigned int d = 0; d < dim; ++d) // Loop over dimension
      {
        const auto power_d = powers[d];
        if (power_d == 0)
          continue;

        // Compute partial derivative in direction d
        Real partial = coef * power_d;

        for (unsigned int i = 0; i < powers.size(); ++i)
        {
          if (i == d)
          {
            if (powers[i] > 1)
              partial *= std::pow(p(i), powers[i] - 1); // reduce power by 1
          }
          else
          {
            if (powers[i] > 0)
              partial *= std::pow(p(i), powers[i]); // full power
          }
        }

        grad(d) += partial;
      }
    }

    return grad;
  };

  _fe_problem.projectFunctionOnCustomRange(reinitializedElemRange(displaced),
                                           reinitializedNodeRangeFromBndNodes(displaced),
                                           poly_func,
                                           poly_func_grad,
                                           function_parameters,
                                           var_name);
}
