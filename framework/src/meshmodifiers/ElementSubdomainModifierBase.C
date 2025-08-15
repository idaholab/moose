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

#include "libmesh/dof_map.h"
#include "libmesh/remote_elem.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/parameters.h"
#include <iterator>
#include <unordered_set>

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
  params.addParam<double>(
      "nearby_distance_threshold",
      -1.0,
      "Threshold for considering elements as 'nearby' in the K-D tree search. Only elements within "
      "this distance will be considered for polynomial fitting.");
  params.addParam<std::vector<bool>>(
      "restore_overridden_dofs",
      {},
      "A list of boolean flags, one for each variable in 'reinitialize_variables', specifying "
      "whether overridden DOF values should be restored after reinitialization for each variable. "
      "This is useful when the solved values on these DOFs should be preserved. If the list is "
      "empty, overridden DOF values will NOT be restored for any variable by default.");

  params.registerBase("MeshModifier");

  return params;
}

ElementSubdomainModifierBase::ElementSubdomainModifierBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _displaced_mesh(_displaced_problem ? &_displaced_problem->mesh() : nullptr),
    _nl_sys(_fe_problem.getNonlinearSystemBase(systemNumber())),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _t_step_old(declareRestartableData<int>("t_step_old", 0)),
    _restep(false),
    _has_done_previous_step(false),
    _old_subdomain_reinitialized(getParam<bool>("old_subdomain_reinitialized")),
    _pr_names(getParam<std::vector<UserObjectName>>("polynomial_fitters")),
    _reinit_vars(getParam<std::vector<VariableName>>("reinitialize_variables")),
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

  // Determine the reinitialization strategy for each variable.
  //   (1) If they are of the same size, we perform a 1-to-1 mapping.
  //   (2) If only one strategy or restore flag is provided, it applies to all variables.
  const auto reinit_strategy_in = getParam<std::vector<MooseEnum>>("reinitialization_strategy");
  const auto restore_overridden_dofs_in = getParam<std::vector<bool>>("restore_overridden_dofs");

  if (std::any_of(reinit_strategy_in.begin(),
                  reinit_strategy_in.end(),
                  [](const MooseEnum & val) { return val == "POLYNOMIAL_NEARBY"; }) &&
      !isParamSetByUser("nearby_distance_threshold"))
    mooseError("The 'nearby_distance_threshold' parameter must be set when using the "
               "POLYNOMIAL_NEARBY reinitialization strategy.");

  if (reinit_strategy_in.size() == 1)
    _reinit_strategy.resize(_reinit_vars.size(), reinit_strategy_in[0].getEnum<ReinitStrategy>());
  else if (reinit_strategy_in.size() == _reinit_vars.size())
    for (const auto & e : reinit_strategy_in)
      _reinit_strategy.push_back(e.getEnum<ReinitStrategy>());
  else
    paramError(
        "reinitialization_strategy",
        "The 'reinitialization_strategy' parameter must have either a single value or a number "
        "of values equal to the number of 'reinitialize_variables'. "
        "Got ",
        reinit_strategy_in.size(),
        " strategies for ",
        _reinit_vars.size(),
        " variables.");

  if (restore_overridden_dofs_in.size() == 1)
  {
    if (restore_overridden_dofs_in[0])
      _vars_to_restore_overridden_dofs =
          _reinit_vars; // Restore overridden DOFs for all reinitialized variables
  }
  else if (restore_overridden_dofs_in.size() == _reinit_vars.size())
  {
    for (auto i : index_range(_reinit_vars))
      if (restore_overridden_dofs_in[i])
        _vars_to_restore_overridden_dofs.push_back(_reinit_vars[i]);
  }
  else
  {
    if (!restore_overridden_dofs_in.empty())
      paramError(
          "restore_overridden_dofs",
          "The 'restore_overridden_dofs' parameter must have either a single value or a number "
          "of values equal to the number of 'reinitialize_variables'. "
          "Got ",
          restore_overridden_dofs_in.size(),
          " restore_overridden_dofs for ",
          _reinit_vars.size(),
          " variables.");
  }

  // For all the other variables, we set the reinitialization strategy to IC
  for (const auto & var_name : _fe_problem.getVariableNames())
    if (std::find(_reinit_vars.begin(), _reinit_vars.end(), var_name) == _reinit_vars.end())
    {
      _reinit_vars.push_back(var_name);
      _reinit_strategy.push_back(ReinitStrategy::IC);
    }

  // Map variable names to the index of the nodal patch recovery user object
  _pr.resize(_pr_names.size());
  std::size_t pr_count = 0;
  for (auto i : index_range(_reinit_vars))
    if (_reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEIGHBOR ||
        _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_WHOLE ||
        _reinit_strategy[i] == ReinitStrategy::POLYNOMIAL_NEARBY)
    {
      _var_name_to_pr_idx[_reinit_vars[i]] = pr_count;
      if (pr_count >= _pr_names.size())
        paramError("polynomial_fitters",
                   "The number of polynomial fitters (",
                   _pr_names.size(),
                   ") is less than the number of variables to reinitialize with polynomial "
                   "extrapolation.");
      _pr[pr_count] =
          &_fe_problem.getUserObject<NodalPatchRecoveryBase>(_pr_names[pr_count], /*tid=*/0);
      _depend_uo.insert(_pr_names[pr_count]);
      pr_count++;
    }
  if (_pr_names.size() != pr_count)
    paramError("polynomial_fitters",
               "Mismatch between number of reinitialization strategies using polynomial "
               "extrapolation and polynomial fitters (expected: ",
               pr_count,
               ", given: ",
               _pr_names.size(),
               ").");
}

void
ElementSubdomainModifierBase::timestepSetup()
{
  // Case 1: The timestep has advanced, and _has_done_previous_step is true.
  //         (This is the normal case; _has_done_previous_step would not be false here.)
  //
  // Case 2: The timestep has not advanced, and _has_done_previous_step is false.
  //         (This means the user intentionally calls ESM multiple times within one timestep,
  //          such as in XFEM. In this case, we should NOT revert the subdomain changes.)
  //
  // Case 3: The timestep has not advanced, and _has_done_previous_step is true, which we should
  //         REVERT the subdomain changes.
  //         There are two possible scenarios here:
  //         (1) A restep (repeating the step in unit tests).
  //         (2) The solution diverged, and we want to retry with a reduced timestep.
  if (_t_step == _t_step_old && _has_done_previous_step)
  {
    mooseInfoRepeated(name(), ": Restoring element subdomain changes.");

    // Reverse the subdomain changes
    auto moved_elem_reversed = _moved_elems;
    for (auto & [elem_id, subdomain] : moved_elem_reversed)
      std::swap(subdomain.first, subdomain.second);

    _restep = true;
    modify(moved_elem_reversed);
    _restep = false;
  }

  _t_step_old = _t_step;

  _has_done_previous_step = false;
}

void
ElementSubdomainModifierBase::modify(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  if (!_restep)
    // Cache the moved elements for potential restore
    _moved_elems = moved_elems;

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

  // Apply cached subdomain changes
  applySubdomainChanges(moved_elems, _mesh);
  if (_displaced_mesh)
    applySubdomainChanges(moved_elems, *_displaced_mesh);

  // Update moving boundaries
  gatherMovingBoundaryChanges(moved_elems);
  applyMovingBoundaryChanges(_mesh);
  if (_displaced_mesh)
    applyMovingBoundaryChanges(*_displaced_mesh);

  // Some variable reinitialization strategies require patch elements to be gathered
  // This has to be done *before* reinitializing the equation systems because we need to find
  // currently evaluable elements
  if (!_restep)
  {
    _evaluable_elems.clear();
    _patch_elem_ids.clear();
    for (auto i : index_range(_reinit_vars))
      prepareVariableForReinitialization(_reinit_vars[i], _reinit_strategy[i]);
  }

  // Reinit equation systems
  _fe_problem.meshChanged(
      /*intermediate_change=*/false, /*contract_mesh=*/false, /*clean_refinement_flags=*/false);

  // Initialize solution and stateful material properties
  if (!_restep)
  {
    applyIC();
    if (_fe_problem.getMaterialWarehouse().hasActiveObjects(0))
      initElementStatefulProps();
    _has_done_previous_step = true; // Indicate that the timestep has been advanced
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
ElementSubdomainModifierBase::prepareVariableForReinitialization(const VariableName & var_name,
                                                                 ReinitStrategy reinit_strategy)
{
  switch (reinit_strategy)
  {
    case ReinitStrategy::IC:
      // No additional preparation needed for IC
      break;
    case ReinitStrategy::POLYNOMIAL_NEIGHBOR:
    case ReinitStrategy::POLYNOMIAL_WHOLE:
    case ReinitStrategy::POLYNOMIAL_NEARBY:
    {
      if (_var_name_to_pr_idx.find(var_name) == _var_name_to_pr_idx.end())
        return;
      const int pr_idx = _var_name_to_pr_idx[var_name];
      // The patch elements might be different for each variable
      gatherPatchElements(var_name, reinit_strategy);

      // Notify the patch recovery user object about the patch elements
      _pr[pr_idx]->sync(_patch_elem_ids[var_name]);

      break;
    }
    default:
      mooseError("Unknown reinitialization strategy");
      break;
  }
}

void
ElementSubdomainModifierBase::meshChanged()
{
  // Clear cached ranges
  _reinitialized_elem_range.reset();
  _reinitialized_bnd_node_range.reset();
  _reinitialized_node_range.reset();

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

  // One more algorithm:
  // (1) Loop over moved elements
  // (2) If neighbor element processor ID is not the same as current processor ID (ghost element),
  //     push the moved element ID to the neighbor processor

  std::unordered_map<processor_id_type, std::unordered_set<dof_id_type>> push_data_set;
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> push_data;

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
    const auto & elem = _mesh.elemPtr(elem_id);

    // (1) Loop over nodes of moved elements
    // (2) node to element map is used to find neighbor elements
    // (3) If neighbor element processor ID is not the same as current processor ID (means that the
    // current element is ghosted element to the neighbor processor), push the moved element (or
    // reinitialized, or newly-activated) ID to the neighbor processor
    for (const auto & node : elem->node_ref_range())
      for (const auto & neigh_id : _mesh.nodeToElemMap().at(node.id()))
        if (neigh_id != elem_id) // Don't check the element itself
        {
          const auto neigh_elem = _mesh.elemPtr(neigh_id);
          if (neigh_elem->processor_id() != processor_id())
            push_data_set[neigh_elem->processor_id()].insert(elem_id);
        }

    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      if (nodeIsNewlyReinitialized(elem->node_id(i)))
        _reinitialized_nodes.insert(elem->node_id(i));
  }

  for (auto & [pid, s] : push_data_set)
    push_data[pid] = {s.begin(), s.end()};

  _semi_local_reinitialized_elems = _reinitialized_elems;

  auto push_receiver =
      [this](const processor_id_type, const std::vector<dof_id_type> & received_data)
  {
    for (const auto & id : received_data)
      _semi_local_reinitialized_elems.insert(id);
  };

  Parallel::push_parallel_vector_data(_mesh.comm(), push_data, push_receiver);
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
ElementSubdomainModifierBase::applyIC()
{
  // Before reinitializing variables, some DOFs may be overwritten.
  // By default, these overwritten DOF values are NOT restored.
  // If the user sets `restore_overridden_dofs` to true, we first save the current
  // values of these DOFs, then restore them after reinitialization.
  for (const auto & var_name : _vars_to_restore_overridden_dofs)
    storeOverriddenDofValues(var_name);

  // ReinitStrategy::IC
  std::set<VariableName> ic_vars;
  for (auto i : index_range(_reinit_vars))
    if (_reinit_strategy[i] == ReinitStrategy::IC)
      ic_vars.insert(_reinit_vars[i]);
  if (!ic_vars.empty())
    _fe_problem.projectInitialConditionOnCustomRange(
        reinitializedElemRange(), reinitializedBndNodeRange(), ic_vars);

  // ReinitStrategy::POLYNOMIAL_NEIGHBOR, POLYNOMIAL_WHOLE, POLYNOMIAL_NEARBY
  for (const auto & [var, patch] : _patch_elem_ids)
    extrapolatePolynomial(var);

  // See the comment above, now we restore the values of the dofs that were overridden
  for (const auto & var_name : _vars_to_restore_overridden_dofs)
    restoreOverriddenDofValues(var_name);

  mooseAssert(_fe_problem.numSolverSystems() <= 1,
              "This code was written for a single nonlinear system");
  // Set old and older solutions on the reinitialized dofs to the reinitialized values
  // note: from current -> old -> older
  setOldAndOlderSolutions(_fe_problem.getNonlinearSystemBase(_sys.number()),
                          reinitializedElemRange(),
                          reinitializedBndNodeRange());
  setOldAndOlderSolutions(
      _fe_problem.getAuxiliarySystem(), reinitializedElemRange(), reinitializedBndNodeRange());

  // Note: Need method to handle solve failures at timesteps where subdomain changes. The old
  // solutions are now set to the reinitialized values. Does this impact restoring solutions
}

void
ElementSubdomainModifierBase::storeOverriddenDofValues(const VariableName & var_name)
{
  const auto & sys = _fe_problem.getSystem(var_name);
  const auto & current_solution = *sys.current_local_solution;
  const auto & dof_map = sys.get_dof_map();
  const auto & var = _fe_problem.getStandardVariable(0, var_name);
  const auto var_num = var.number();

  // Get the DOFs on the reinitialized elements
  // Here we should loop over both ghosted and local reinitialized elements.
  // The ghosted elements here can take care of DoFs that is belong to the reinitialized
  // elements but are not on the current processor.
  std::set<dof_id_type> reinitialized_dofs;
  for (const auto & elem_id : _semi_local_reinitialized_elems)
  {
    const auto & elem = _mesh.elemPtr(elem_id);
    std::vector<dof_id_type> elem_dofs;
    dof_map.dof_indices(elem, elem_dofs, var_num);
    reinitialized_dofs.insert(elem_dofs.begin(), elem_dofs.end());
  }

  // Get existing DOFs on the active elements excluding reinitialized elements
  std::set<dof_id_type> existing_dofs;
  for (const auto * elem : *_mesh.getActiveLocalElementRange())
  {
    if (_reinitialized_elems.count(elem->id()))
      continue; // Skip reinitialized elements
    std::vector<dof_id_type> elem_dofs;
    dof_map.dof_indices(elem, elem_dofs, var_num);
    existing_dofs.insert(elem_dofs.begin(), elem_dofs.end());
  }

  // Get the DOFs on the nodes that are overridden on reinitialized elements
  std::vector<dof_id_type> overridden_dofs;
  std::set_intersection(reinitialized_dofs.begin(),
                        reinitialized_dofs.end(),
                        existing_dofs.begin(),
                        existing_dofs.end(),
                        std::back_inserter(overridden_dofs));

  // Values before overriding (to be restored later)
  std::vector<Number> values;
  for (auto dof : overridden_dofs)
    values.push_back(current_solution(dof));

  _overridden_values_on_reinit_elems[var_name] = {overridden_dofs, values};
}

void
ElementSubdomainModifierBase::restoreOverriddenDofValues(const VariableName & var_name)
{
  const auto sn = _fe_problem.systemNumForVariable(var_name);
  auto & sys = _fe_problem.getSystemBase(sn);
  auto & sol = sys.solution();
  const auto & dof_map = sys.dofMap();
  const auto & [dof_ids, values] = _overridden_values_on_reinit_elems[var_name];

  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, Number>>> push_data;

  for (const int i : index_range(dof_ids))
  {
    if (dof_map.dof_owner(dof_ids[i]) == processor_id())
      sol.set(dof_ids[i], values[i]);
    else
      push_data[dof_map.dof_owner(dof_ids[i])].emplace_back(dof_ids[i], values[i]);
  }

  auto push_receiver = [&](const processor_id_type,
                           const std::vector<std::pair<dof_id_type, Number>> & received_data)
  {
    for (const auto & [id, value] : received_data)
      sol.set(id, value);
  };

  Parallel::push_parallel_vector_data(_mesh.comm(), push_data, push_receiver);

  sol.close();
  sol.localize(*sys.system().current_local_solution, sys.dofMap().get_send_list());
}

void
ElementSubdomainModifierBase::initElementStatefulProps()
{
  _fe_problem.initElementStatefulProps(reinitializedElemRange(), /*threaded=*/true);
}

ConstElemRange &
ElementSubdomainModifierBase::reinitializedElemRange()
{
  if (_reinitialized_elem_range)
    return *_reinitialized_elem_range.get();

  // Create a vector of the newly reinitialized elements
  std::vector<Elem *> elems;
  for (auto elem_id : _reinitialized_elems)
    elems.push_back(_mesh.elemPtr(elem_id));

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  _reinitialized_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);

  return reinitializedElemRange();
}

ConstBndNodeRange &
ElementSubdomainModifierBase::reinitializedBndNodeRange()
{
  if (_reinitialized_bnd_node_range)
    return *_reinitialized_bnd_node_range.get();

  // Create a vector of the newly reinitialized boundary nodes
  std::vector<const BndNode *> nodes;
  auto bnd_nodes = _mesh.getBoundaryNodeRange();
  for (auto bnd_node : *bnd_nodes)
    if (bnd_node->_node)
      if (_reinitialized_nodes.count(bnd_node->_node->id()))
        nodes.push_back(bnd_node);

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  _reinitialized_bnd_node_range =
      std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);

  return reinitializedBndNodeRange();
}

ConstNodeRange &
ElementSubdomainModifierBase::reinitializedNodeRange()
{
  if (_reinitialized_node_range)
    return *_reinitialized_node_range.get();

  // Create a vector of the newly reinitialized nodes
  std::vector<const Node *> nodes;

  for (auto node_id : _reinitialized_nodes)
    nodes.push_back(_mesh.nodePtr(node_id)); // displaced mesh shares the same node object

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
ElementSubdomainModifierBase::gatherPatchElements(const VariableName & var_name,
                                                  ReinitStrategy reinit_strategy)
{
  _patch_elem_ids[var_name].clear();

  // First collect all elements who own dofs in the current dofmap
  auto & sys = _fe_problem.getSystem(var_name);

  // Cache evaluable elements for the system if not already done
  if (!_evaluable_elems.count(sys.number()))
  {
    auto & [candidate_elems, candidate_elem_ids] = _evaluable_elems[sys.number()];
    const auto & dof_map = sys.get_dof_map();
    std::vector<dof_id_type> elem_dofs;
    auto vn = sys.variable_number(static_cast<std::string>(var_name));
    for (const auto elem : *_mesh.getActiveLocalElementRange())
    {
      if (std::find(_reinitialized_elems.begin(), _reinitialized_elems.end(), elem->id()) !=
          _reinitialized_elems.end())
        continue; // Skip elements that were reinitialized

      dof_map.dof_indices(elem, elem_dofs, vn);
      if (!elem_dofs.empty())
      {
        candidate_elems.insert(elem);
        candidate_elem_ids.push_back(elem->id());
      }
    }
  }
  auto & [candidate_elems, candidate_elem_ids] = _evaluable_elems[sys.number()];

  // Now we gather patch elements based on the reinit strategy
  auto & patch_elems = _patch_elem_ids[var_name];

  switch (reinit_strategy)
  {
    case ReinitStrategy::POLYNOMIAL_NEIGHBOR:
    {
      auto has_neighbor_in_reinit_elems = [&](const Elem * elem) -> bool
      {
        for (const auto & node : elem->node_ref_range())
          for (const auto & neigh_id : _mesh.nodeToElemMap().at(node.id()))
            // here we need to use _global_reinitialized_elems gathering from all processors
            if (_semi_local_reinitialized_elems.count(neigh_id))
              return true;
        return false;
      };
      // Loop over all candidate elements, for each element, if any of its point neighbor belongs
      // to the reinitialized elements, we will include that element in the patch element set.
      for (const auto * elem : candidate_elems)
        if (has_neighbor_in_reinit_elems(elem))
          patch_elems.push_back(elem->id());
      break;
    }
    case ReinitStrategy::POLYNOMIAL_WHOLE:
    {
      // This is simple: all candidate elements are patch elements
      patch_elems = candidate_elem_ids;
      break;
    }
    case ReinitStrategy::POLYNOMIAL_NEARBY:
    {
      std::vector<Point> kd_points;
      std::vector<dof_id_type> global_candidate_elem_ids;

      if (_mesh.isDistributedMesh())
      {
        std::vector<std::pair<Point, dof_id_type>> pts_ids(candidate_elem_ids.size());
        for (std::size_t i = 0; i < candidate_elem_ids.size(); ++i)
          pts_ids[i] = {_mesh.elemPtr(candidate_elem_ids[i])->vertex_average(),
                        candidate_elem_ids[i]};
        _mesh.comm().allgather(pts_ids);
        for (const auto & [pt, id] : pts_ids)
        {
          kd_points.push_back(pt);
          global_candidate_elem_ids.push_back(id);
        }
      }
      else
      {
        _mesh.comm().allgather(candidate_elem_ids);
        global_candidate_elem_ids = candidate_elem_ids;
        for (const auto & id : candidate_elem_ids)
          kd_points.push_back(_mesh.elemPtr(id)->vertex_average());
      }

      const auto kd_tree = std::make_unique<KDTree>(kd_points, _leaf_max_size);

      std::vector<nanoflann::ResultItem<std::size_t, Real>> query_result;
      for (const auto & elem_id : _reinitialized_elems)
      {
        const Point & centroid = _mesh.elemPtr(elem_id)->vertex_average();
        kd_tree->radiusSearch(centroid, _nearby_distance_threshold, query_result);
        for (const auto & [qid, dist] : query_result)
          patch_elems.push_back(global_candidate_elem_ids[qid]);
      }
      break;
    }
    default:
      mooseError("Unknown reinitialization strategy");
      break;
  }

  // every processor should have the same patch elements to do the polynomial extrapolation,
  // so we gather them across all processors
  _mesh.comm().allgather(patch_elems);

  // Remove duplicates from the patch elements (espcially important for POLYNOMIAL_NEARBY)
  std::sort(patch_elems.begin(), patch_elems.end());
  patch_elems.erase(std::unique(patch_elems.begin(), patch_elems.end()), patch_elems.end());
}

void
ElementSubdomainModifierBase::extrapolatePolynomial(const VariableName & var_name)
{
  const auto & coef =
      _pr[_var_name_to_pr_idx[var_name]]->getCoefficients(_patch_elem_ids[var_name]);

  const unsigned dim = _mesh.dimension();

  libMesh::Parameters function_parameters;

  const auto & multi_index = _pr[_var_name_to_pr_idx[var_name]]->multiIndex();

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
      for (unsigned int d = 0; d < multi_index[r].size(); d++)
      {
        const auto power = multi_index[r][d];
        if (power == 0)
          continue;

        monomial *= std::pow(p(d), power);
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

  _fe_problem.projectFunctionOnCustomRange(
      reinitializedElemRange(), poly_func, poly_func_grad, function_parameters, var_name);
}
