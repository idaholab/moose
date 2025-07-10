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

  params.addDeprecatedParam<BoundaryName>(
      "moving_boundary_name",
      "Name of the moving boundary",
      "This has been replaced by 'moving_boundaries' and 'moving_boundary_subdomain_pairs'.");
  params.addDeprecatedParam<BoundaryName>(
      "complement_moving_boundary_name",
      "Name of the moving boundary on the complement subdomain(s)",
      "This has been replaced by 'moving_boundaries' and 'moving_boundary_subdomain_pairs'.");
  params.addDeprecatedParam<bool>(
      "apply_initial_conditions",
      true,
      "Whether to apply initial conditions on the moved nodes and elements",
      "This has been replaced by 'reinitialize_subdomains'");

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

  params.addParam<std::vector<SubdomainID>>(
      "reinitialize_subdomain_ids",
      {},
      "List of subdomain IDs to reinitialize. If empty, all subdomains will be reinitialized.");

  params.addParam<bool>(
      "old_subdomain_reinitialized",
      true,
      "This parameter must be set with a non-empty list in 'reinitialize_subdomains'. When set to "
      "the default true, the element's old subdomain is not considered when determining if an "
      "element should be reinitialized. If set to false, only elements whose old subdomain was not "
      "in 'reinitialize_subdomains' are reinitialized. ");

  params.addParam<std::vector<VariableName>>(
      "ic_variables", {}, "Which variables to set IC on the newly activated nodes. ");

  MooseEnum ic_strategy("DEFAULT POLYNOMIAL "
                        "POLYNOMIAL_WHOLE_SOLVED_DOMAIN POLYNOMIAL_THRESHOLD",
                        "DEFAULT");

  params.addParam<std::vector<MooseEnum>>(
      "ic_strategy",
      {ic_strategy},
      "The strategy to set the initial condition on the newly activated nodes. ");

  params.addParam<std::vector<UserObjectName>>(
      "nodal_patch_recovery_uo",
      {},
      "List of NodalPatchRecovery UserObjects for each component (e.g., u, v)");

  params.addParam<int>(
      "kd_tree_leaf_max_size", 10, "Maximum number of elements in a leaf node of the k-d tree");

  params.addParam<int>("nearby_element_threshold",
                       1,
                       "Threshold for considering elements as 'nearby' in the k-d tree search.");

  params.addParam<double>("radius_search_threshold",
                          -1.0,
                          "Threshold for considering elements as 'nearby' in the k-d tree search.");

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
    _npr_names(getParam<std::vector<UserObjectName>>("nodal_patch_recovery_uo")),
    _npr_vec(_npr_names.size()),
    _ic_vars_names(getParam<std::vector<VariableName>>("ic_variables")),
    _nearby_element_threshold(getParam<int>("nearby_element_threshold")),
    _leaf_max_size(getParam<int>("kd_tree_leaf_max_size")),
    _radius_search_threshold(getParam<double>("radius_search_threshold"))
{
  // Check if the variables to set IC exist in the system
  for (auto var_name : _ic_vars_names)
    if (!_nl_sys.hasVariable(var_name) && !_aux_sys.hasVariable(var_name))
      paramError("ic_variables", "Variable ", var_name, " does not exist.");

  for (const auto & i : index_range(_npr_names))
    _npr_vec[i] = &getUserObjectByName<NodalPatchRecoveryBase>(_npr_names[i]);

  // Ensure that the size of _ic_strategy is either 1 or matches the number of variables to
  // initialize
  const auto ic_strategy_in = getParam<std::vector<MooseEnum>>("ic_strategy");

  /**
   * Determine the IC strategy for each variable to initialize.
   *
   * The logic follows these cases:
   * (a) If both `ic_strategy` and `ic_variables` are provided and of the same size,
   *     we perform a 1-to-1 mapping.
   * (b) If `ic_strategy` is set, but `ic_variables` is empty, and `nodal_patch_recovery_uo` is set:
   *     (b1) If sizes match -> 1-to-1 mapping.
   *     (b2) If only one strategy is provided -> apply it to all variables from NPR.
   *     (b3) Otherwise -> error.
   * (c) If `ic_strategy` is set, but both `ic_variables` and NPR are empty:
   *     (c1) If one component in `ic_strategy` == POLYNOMIAL... -> mooseError
   *     (c2) If one strategy is provided (basically only work for `DEFAULT`) -> apply to all
   *      nonlinear variables.
   *     (c3) If more than one strategy is provided -> warning + use first for all.
   * (d) If `ic_strategy` has one value and `ic_variables` is provided -> apply to all
   * `ic_variables`.
   * (e) Otherwise -> raise a parameter error.
   */
  if (!_ic_vars_names.empty() && _ic_vars_names.size() == ic_strategy_in.size())
    for (const auto & e : ic_strategy_in)
      _ic_strategy.push_back(e.getEnum<ICStrategy>());
  else if (!ic_strategy_in.empty() && _ic_vars_names.empty() && !_npr_vec.empty())
  {
    // get variable names from the npr vector
    _ic_vars_names.resize(_npr_vec.size());
    for (const int i : index_range(_npr_vec))
      _ic_vars_names[i] = _npr_vec[i]->variableName();

    if (ic_strategy_in.size() == _npr_vec.size())
      for (const auto & e : ic_strategy_in)
        _ic_strategy.push_back(e.getEnum<ICStrategy>());
    else if (ic_strategy_in.size() == 1)
      _ic_strategy.resize(_npr_vec.size(), ic_strategy_in[0].getEnum<ICStrategy>());
    else
      paramError(
          "ic_strategy",
          "The 'ic_strategy' parameter must have either a single value or a number of "
          "values equal to the number of nodal_patch_recovery_uo specified in `[MeshModifiers]`.");
  }
  else if (!ic_strategy_in.empty() && _ic_vars_names.empty()) /*_npr_vec is empty*/
  {
    bool ic_has_polynomial = false;
    for (const auto & e : ic_strategy_in)
    {
      const auto ic_enum = e.getEnum<ICStrategy>();
      if (ic_enum == ICStrategy::POLYNOMIAL ||
          ic_enum == ICStrategy::POLYNOMIAL_WHOLE_SOLVED_DOMAIN ||
          ic_enum == ICStrategy::POLYNOMIAL_THRESHOLD)
      {
        ic_has_polynomial = true;
        break;
      }
    }
    if (ic_has_polynomial)
      mooseError(
          "The 'ic_strategy' parameter is set to use polynomial extrapolation, but no "
          "'nodal_patch_recovery_uo' is specified. Please specify at least one NodalPatchRecovery "
          "UserObject to use polynomial extrapolation.");

    // get variable names from the non-linear system
    if (ic_strategy_in.size() > 1)
      mooseWarning("The 'ic_strategy' parameter has more than one value, but no 'ic_variables' are "
                   "specified. "
                   "This will apply the same strategy to all variables in the non-linear system "
                   "based on your first value in ic_strategy.");
    _ic_strategy.resize(_nl_sys.nVariables(), ic_strategy_in[0].getEnum<ICStrategy>());
    _ic_vars_names = _nl_sys.getVariableNames();
  }
  else if (!_ic_vars_names.empty() && ic_strategy_in.size() == 1)
    _ic_strategy.resize(_ic_vars_names.size(), ic_strategy_in[0].getEnum<ICStrategy>());
  else
    paramError("ic_strategy",
               "The 'ic_strategy' parameter is not used correctly. Please follow one of the "
               "supported forms:\n"
               " - Provide a single value to apply the same IC strategy to all variables.\n"
               " - Provide the same number of values as 'ic_variables' to assign strategies "
               "individually.\n"
               " - If using 'nodal_patch_recovery_uo', the number of strategies must be 1 or equal "
               "to the number of NPR objects.\n"
               "Ensure that either 'ic_variables' or 'nodal_patch_recovery_uo' is specified for "
               "multiple strategies.");

  if (!_npr_vec.empty())
  {
    // Count how many variables use NPR strategy
    unsigned int npr_count = 0;

    for (auto i : index_range(_ic_vars_names))
    {
      if (_ic_strategy[i] == ICStrategy::POLYNOMIAL ||
          _ic_strategy[i] == ICStrategy::POLYNOMIAL_WHOLE_SOLVED_DOMAIN ||
          _ic_strategy[i] == ICStrategy::POLYNOMIAL_THRESHOLD)
      {
        _var_name_to_npr_idx[_ic_vars_names[i]] = npr_count;
        npr_count++;
      }
    }

    // Check size of npr_vec is equal to npr_count
    if (_npr_vec.size() != npr_count)
      mooseError("Mismatch between number of IC strategies using polynomial "
                 "extrapolation and NPR UO (expected: " +
                 std::to_string(npr_count) + ", given: " + std::to_string(_npr_vec.size()) + ").");

    _solved_elem_ids_for_npr.resize(_npr_vec.size());
  }

  if (isParamSetByUser("moving_boundary_name") ||
      isParamSetByUser("complement_moving_boundary_name"))
    mooseError(
        "'moving_boundary_name' and 'complement_moving_boundary_name' have been replaced by "
        "'moving_boundaries' and 'moving_boundary_subdomain_pairs'. See the documentation in "
        "https://mooseframework.inl.gov/blackbear/source/userobjects/"
        "ElementSubdomainModifier.html for more information. "
        "Additionally, SidesetAroundSubdomainUpdater can now be used to update boundaries "
        "that are defined around a subdomain, or between two subdomains.");
}

void
ElementSubdomainModifierBase::initialSetup()
{
  // Error if both parameters are set
  if (isParamSetByUser("apply_initial_conditions") &&
      isParamSetByUser("reinitialize_subdomain_ids"))
    mooseError("Cannot set both 'apply_initial_conditions' and 'reinitialize_subdomain_ids'. "
               "Please use only one.");

  // When 'apply_initial_conditions' is fully deprecated, change this to 'const' vector
  std::vector<SubdomainName> subdomain_names_to_reinitialize =
      getParam<std::vector<SubdomainName>>("reinitialize_subdomains");

  // When 'apply_initial_conditions' is fully deprecated, remove this block
  if (isParamSetByUser("apply_initial_conditions"))
  {
    if (getParam<bool>("apply_initial_conditions"))
      subdomain_names_to_reinitialize = {"ANY_BLOCK_ID"};
    else
      subdomain_names_to_reinitialize = {};
  }

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

  if (!_npr_vec.empty())
    for (auto i : index_range(_ic_vars_names))
    {
      const auto var_name = _ic_vars_names[i];
      if (_var_name_to_npr_idx.find(var_name) == _var_name_to_npr_idx.end())
        continue;

      const int npr_idx = _var_name_to_npr_idx[var_name];

      gatherNeighborElementsForActivatedNodes(i);
      _npr_vec[npr_idx]->cacheAdditionalElements(_solved_elem_ids_for_npr[npr_idx]);
      _npr_vec[npr_idx]->identifyAdditionalElementsFromOtherProcs();
      _npr_vec[npr_idx]->synchronizeAebe();
      _npr_vec[npr_idx]->cleanQueryIDsAndAdditionalElements();
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

  std::unordered_set<dof_id_type> local_reinitialized_nodes;

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
    }
  }

  // Convert to vector and allgather across processors
  std::vector<dof_id_type> local(local_reinitialized_nodes.begin(),
                                 local_reinitialized_nodes.end());

  std::vector<std::vector<dof_id_type>> gathered;
  _mesh.comm().allgather(local, gathered);

  // Collect globally unique activated nodes
  std::unordered_set<dof_id_type> unique_nodes;
  for (const auto & vec : gathered)
    unique_nodes.insert(vec.begin(), vec.end());

  // Filters the globally reinitialized nodes and stores only those owned by this processor.
  for (auto id : unique_nodes)
  {
    const auto node = _mesh.nodePtr(id);
    if (node->processor_id() == _mesh.processor_id())
      _reinitialized_nodes.insert(id);
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
  // Set of variable names that are not part of the extrapolated initial conditions
  std::set<VariableName> ic_target_vars_names_except_ic_vars;

  auto insertNonICVars = [&](SystemBase & sys) -> void
  {
    const auto & vars = sys.getVariables(_tid);
    for (const auto & ivar : vars)
      if (std::find(_ic_vars_names.begin(), _ic_vars_names.end(), ivar->name()) ==
          _ic_vars_names.end())
        ic_target_vars_names_except_ic_vars.insert(ivar->name());
  };

  insertNonICVars(_nl_sys);
  insertNonICVars(_aux_sys);

  // note: from IC -> current
  _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                   reinitializedBndNodeRange(displaced),
                                                   TargetVarUsageForIC::ONLY_LIST,
                                                   ic_target_vars_names_except_ic_vars);

  // Loop over each variable and initialize
  for (auto i : index_range(_ic_vars_names))
  {
    if (_ic_strategy[i] == ICStrategy::DEFAULT)
      // note: from IC -> current
      _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                       reinitializedBndNodeRange(displaced),
                                                       TargetVarUsageForIC::ONLY_LIST,
                                                       {_ic_vars_names[i]});
    else if (_ic_strategy[i] == ICStrategy::POLYNOMIAL ||
             _ic_strategy[i] == ICStrategy::POLYNOMIAL_WHOLE_SOLVED_DOMAIN ||
             _ic_strategy[i] == ICStrategy::POLYNOMIAL_THRESHOLD)
    {
      projectNprIC(_ic_vars_names[i], displaced);
    }
    else
      mooseError("Unknown initial condition strategy");
  }

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
ElementSubdomainModifierBase::gatherNeighborElementsForActivatedNodes(const unsigned int ic_idx)
{

  auto local2Global =
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

  auto local2GlobalPair =
      [&](const auto & local_vals1,
          const auto & local_vals2,
          std::vector<typename std::decay<decltype(local_vals1[0])>::type> & global_vals1,
          std::vector<typename std::decay<decltype(local_vals2[0])>::type> & global_vals2) -> void
  {
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

  if (_ic_strategy[ic_idx] != ICStrategy::POLYNOMIAL_WHOLE_SOLVED_DOMAIN &&
      _ic_strategy[ic_idx] != ICStrategy::POLYNOMIAL &&
      _ic_strategy[ic_idx] != ICStrategy::POLYNOMIAL_THRESHOLD)
    return;

  const auto ic_var_name = _ic_vars_names[ic_idx];
  _solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]].clear();

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

  if (_ic_strategy[ic_idx] == ICStrategy::POLYNOMIAL_THRESHOLD)
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

    local2GlobalPair(
        local_centroids, local_elem_ids, _centroids_of_elements, _kd_tree_sequence_elem_id_map);

    _kd_tree = new KDTree(_centroids_of_elements, _leaf_max_size);

    _mesh.comm().min(_min_diag_length); // TIMPI

    if (_radius_search_threshold < 0.0)
      _radius_search_threshold = _nearby_element_threshold * _min_diag_length;

    for (const auto & elem_id : _global_reinitialized_elems)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      if (!elem)
        continue;

      const Point & centroid = elem->vertex_average();

      std::vector<nanoflann::ResultItem<std::size_t, Real>> matches;

      _kd_tree->radiusSearch(centroid, _radius_search_threshold, matches);

      for (const auto & m : matches)
      {
        dof_id_type neighbor_elem_id =
            _kd_tree_sequence_elem_id_map[static_cast<unsigned int>(m.first)];

        // Skip if already added to patch
        if (patch_elem_set.count(neighbor_elem_id))
          continue;

        _solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]].push_back(neighbor_elem_id);
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

      if (_ic_strategy[ic_idx] == ICStrategy::POLYNOMIAL)
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
          _solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]].push_back(eid);
        }
      }
      else if (_ic_strategy[ic_idx] == ICStrategy::POLYNOMIAL_WHOLE_SOLVED_DOMAIN)
      {
        patch_elem_set.insert(eid);
        _solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]].push_back(eid);
      }
    }
  }

  local2Global(_solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]],
               _solved_elem_ids_for_npr[_var_name_to_npr_idx[ic_var_name]],
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
ElementSubdomainModifierBase::projectNprIC(const VariableName & var_name, bool displaced)
{
  const auto & coef = _npr_vec[_var_name_to_npr_idx[var_name]]->getCoefficients(
      _solved_elem_ids_for_npr[_var_name_to_npr_idx[var_name]]);

  const unsigned dim = _mesh.dimension();

  libMesh::Parameters function_parameters;

  // Coefficient order depends on the problem dimension:
  // - 1D (x):       [c], [c, x], [c, x, x^2]
  // - 2D (x, y):    [c], [c, y, x], [c, y, x, y^2, xy, x^2]
  // - 3D (x, y, z): [c], [c, z, y, x], [c, z, y, x, z^2, zy, zx, y^2, yx, x^2]
  // Terms not included are assumed zero.

  // Set coefficients to parameters with default = 0
  auto get = [&](int i) -> Real { return (i < coef.size()) ? coef(i) : 0.0; };

  function_parameters.set<int>("dimension_for_projection") = dim;
  function_parameters.set<Real>("coef_c") = get(0);

  if (dim == 1)
  {
    function_parameters.set<Real>("coef_x") = get(1);
    function_parameters.set<Real>("coef_xx") = get(2);
  }
  else if (dim == 2)
  {
    function_parameters.set<Real>("coef_y") = get(1);
    function_parameters.set<Real>("coef_x") = get(2);
    function_parameters.set<Real>("coef_yy") = get(3);
    function_parameters.set<Real>("coef_yx") = get(4);
    function_parameters.set<Real>("coef_xx") = get(5);
  }
  else if (dim == 3)
  {
    function_parameters.set<Real>("coef_z") = get(1);
    function_parameters.set<Real>("coef_y") = get(2);
    function_parameters.set<Real>("coef_x") = get(3);
    function_parameters.set<Real>("coef_zz") = get(4);
    function_parameters.set<Real>("coef_zy") = get(5);
    function_parameters.set<Real>("coef_zx") = get(6);
    function_parameters.set<Real>("coef_yy") = get(7);
    function_parameters.set<Real>("coef_yx") = get(8);
    function_parameters.set<Real>("coef_xx") = get(9);
  }

  // Define projection function
  auto poly_func = [](const Point & p,
                      const libMesh::Parameters & parameters,
                      const std::string &,
                      const std::string &) -> libMesh::Number
  {
    const int dim = parameters.get<int>("dimension_for_projection");

    const Real x = p(0);
    const Real y = (dim > 1) ? p(1) : 0;
    const Real z = (dim > 2) ? p(2) : 0;

    Real val = parameters.get<Real>("coef_c");

    if (dim == 1)
      val += parameters.get<Real>("coef_x") * x + parameters.get<Real>("coef_xx") * x * x;
    else if (dim == 2)
      val += parameters.get<Real>("coef_y") * y + parameters.get<Real>("coef_x") * x +
             parameters.get<Real>("coef_yy") * y * y + parameters.get<Real>("coef_yx") * x * y +
             parameters.get<Real>("coef_xx") * x * x;
    else if (dim == 3)
      val += parameters.get<Real>("coef_z") * z + parameters.get<Real>("coef_y") * y +
             parameters.get<Real>("coef_x") * x + parameters.get<Real>("coef_zz") * z * z +
             parameters.get<Real>("coef_zy") * z * y + parameters.get<Real>("coef_zx") * z * x +
             parameters.get<Real>("coef_yy") * y * y + parameters.get<Real>("coef_yx") * y * x +
             parameters.get<Real>("coef_xx") * x * x;

    return val;
  };

  // Define gradient
  auto poly_func_grad = [](const Point & p,
                           const libMesh::Parameters & parameters,
                           const std::string &,
                           const std::string &) -> libMesh::Gradient
  {
    const int dim = parameters.get<int>("dimension_for_projection");
    const Real x = p(0);
    const Real y = (dim > 1) ? p(1) : 0;
    const Real z = (dim > 2) ? p(2) : 0;

    libMesh::Gradient grad;

    if (dim == 1)
      grad(0) = parameters.get<Real>("coef_x") + 2 * parameters.get<Real>("coef_xx") * x;
    else if (dim == 2)
    {
      grad(0) = parameters.get<Real>("coef_x") + parameters.get<Real>("coef_yx") * y +
                2 * parameters.get<Real>("coef_xx") * x;
      grad(1) = parameters.get<Real>("coef_y") + parameters.get<Real>("coef_yx") * x +
                2 * parameters.get<Real>("coef_yy") * y;
    }
    else if (dim == 3)
    {
      grad(0) = parameters.get<Real>("coef_x") + parameters.get<Real>("coef_zx") * z +
                parameters.get<Real>("coef_yx") * y + 2 * parameters.get<Real>("coef_xx") * x;
      grad(1) = parameters.get<Real>("coef_y") + parameters.get<Real>("coef_zy") * z +
                parameters.get<Real>("coef_yx") * x + 2 * parameters.get<Real>("coef_yy") * y;
      grad(2) = parameters.get<Real>("coef_z") + parameters.get<Real>("coef_zy") * y +
                parameters.get<Real>("coef_zx") * x + 2 * parameters.get<Real>("coef_zz") * z;
    }

    return grad;
  };

  _fe_problem.projectFunctionOnCustomRange(reinitializedElemRange(displaced),
                                           reinitializedNodeRangeFromBndNodes(displaced),
                                           reinitializedNodeRange(),
                                           poly_func,
                                           poly_func_grad,
                                           function_parameters,
                                           TargetVarUsageForIC::ONLY_LIST,
                                           {var_name});
}
