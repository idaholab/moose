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
  params.addParam<bool>(
      "old_subdomain_reinitialized",
      true,
      "This parameter must be set with a non-empty list in 'reinitialize_subdomains'. When set to "
      "the default true, the element's old subdomain is not considered when determining if an "
      "element should be reinitialized. If set to false, only elements whose old subdomain was not "
      "in 'reinitialize_subdomains' are reinitialized. ");

  params.addParam<std::string>(
      "ic_strategy",
      "default_value",
      "The strategy to set the initial condition on the newly activated elements. ");

  params.addParam<int>(
      "inactive_subdomain_ID",
      -1,
      "If the region has the inative element, you should set this parameter to turn on the "
      "extrapolation of the solution to the newly activated elements. ");

  params.addParam<std::vector<UserObjectName>>(
      "nodal_patch_recovery_uo",
      {},
      "List of NodalPatchRecovery UserObjects for each component (e.g., u, v)");

  params.registerBase("MeshModifier");

  return params;
}

ElementSubdomainModifierBase::ElementSubdomainModifierBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _displaced_mesh(_displaced_problem ? &_displaced_problem->mesh() : nullptr),
    _old_subdomain_reinitialized(getParam<bool>("old_subdomain_reinitialized")),
    _ic_strategy_string(getParam<std::string>("ic_strategy")),
    _ic_strategy(parseString2ICStrategy(_ic_strategy_string)),
    _inactive_subdomain_ID(getParam<int>("inactive_subdomain_ID")),
    _npr_names(getParam<std::vector<UserObjectName>>("nodal_patch_recovery_uo")),
    _npr_vec(_npr_names.size())
{
  if (_ic_strategy != ICStrategyForNewlyActivated::IC_DEFAULT and _inactive_subdomain_ID == -1)
    mooseError("The inactive subdomain ID must be set to use the extrapolation strategy.");
  if (_ic_strategy == ICStrategyForNewlyActivated::IC_DEFAULT and _inactive_subdomain_ID != -1)
    mooseError("The inactive subdomain ID should not be set to use the default strategy.");
  if (_ic_strategy == ICStrategyForNewlyActivated::IC_POLYNOMIAL and
      (_inactive_subdomain_ID == -1 or _npr_vec.empty()))
    mooseError(
        "The inactive subdomain ID and the NodalPatchRecovery UserObject must be set to use the "
        "polynomial strategy.");

  for (const auto & i : index_range(_npr_names))
    _npr_vec[i] = &getUserObjectByName<NodalPatchRecoveryBase>(_npr_names[i]);

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
  // When 'apply_initial_conditions' is fully deprecated, change this to 'const' vector
  std::vector<SubdomainName> subdomain_names_to_reinitialize =
      getParam<std::vector<SubdomainName>>("reinitialize_subdomains");

  // When 'apply_initial_conditions' is fully deprecated, remove this block
  if (isParamSetByUser("apply_initial_conditions") && getParam<bool>("apply_initial_conditions"))
    subdomain_names_to_reinitialize = {"ANY_BLOCK_ID"};
  else if (isParamSetByUser("apply_initial_conditions"))
    subdomain_names_to_reinitialize = {};

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

  if (_inactive_subdomain_ID != -1)
  {
    identifyFirstPassActivatedNodes(moved_elems);
    identifyLocallyOwnedActivatedNodes(moved_elems);
    computeSetDifference();
    findMissingNewlyActivatedNodes();
  }

  // Reinit equation systems
  _fe_problem.meshChanged();

  // important
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
          std::vector<const Elem *> active_neighs;
          neigh->top_parent()->active_family_tree_by_neighbor(active_neighs, elem);
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

  for (const auto & [elem_id, subdomain] : moved_elems)
  {
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
      {
        _reinitialized_nodes.insert(elem->node_id(i));
      }
    }
  }
}

void
ElementSubdomainModifierBase::identifyFirstPassActivatedNodes(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  // Clear cached element reinitialization data
  _first_pass_local_activated_nodes.clear();

  for (const auto & [elem_id, subdomain] : moved_elems)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);

    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
    {
      const dof_id_type node_id = elem->node_id(i);

      // Skip if not newly activated
      if (!nodeIsNewlyActivated(node_id))
        continue;

      // Insert if not already in set
      _first_pass_local_activated_nodes.insert(node_id);
    }
  }

  gatherFirstPassActivatedNodesGlobally();
}

void
ElementSubdomainModifierBase::identifyLocallyOwnedActivatedNodes(
    const std::unordered_map<dof_id_type, std::pair<SubdomainID, SubdomainID>> & moved_elems)
{
  // Clear cached element reinitialization data
  _newactivated_nodes.clear();

  for (const auto & [elem_id, subdomain] : moved_elems)
  {
    const Elem * elem = _mesh.elemPtr(elem_id);

    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
    {
      const dof_id_type node_id = elem->node_id(i);
      const Node * node = _mesh.nodePtr(node_id);

#ifndef NDEBUG
      if (node->processor_id() != _mesh.processor_id() && nodeIsNewlyActivated(node_id))
      {
        std::cout << "node_id = " << node_id
                  << " is owned by other processor and is newly activated.\n";
      }
#endif

      // Skip if node is not owned by this processor (the main difference with
      // identifyFirstPassActivatedNodes)
      if (node->processor_id() != _mesh.processor_id())
        continue;
      // this previously cause a bug given that the node is not owned by this processor, but also
      // not belong to any moved_elems

      // Skip if not newly activated
      if (!nodeIsNewlyActivated(node_id))
        continue;

      // Insert if not already in set
      _newactivated_nodes.insert(node_id);
    }
  }

  gatherFinalActivatedNodesGlobally();
#ifndef NDEBUG
  if (_newactivated_nodes.size() > 0)
  {
    std::cout << "New activated nodes: ";
    for (auto node_id : _newactivated_nodes)
      std::cout << node_id << " ";
    std::cout << "\n";
  }
#endif
}

void
ElementSubdomainModifierBase::findMissingNewlyActivatedNodes()
{
  for (auto id : _final_global_activated_nodes_diff)
  {
    auto node = _mesh.nodePtr(id);
    if (node->processor_id() == _mesh.processor_id())
    {
#ifndef NDEBUG
      std::cout << "[missing]" << "Rank " << _mesh.processor_id() << " node " << id << "\n";
#endif
      _newactivated_nodes.insert(id);
    }
  }

  gatherFinalActivatedNodesGlobally();
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
  // If any of the node neighbors are already reinitialized, then the node is NOT newly
  // reinitialized.
  for (auto neighbor_elem_id : _mesh.nodeToElemMap().at(node_id))
    if (subdomainIsReinitialized(_mesh.elemPtr(neighbor_elem_id)->subdomain_id()))
      return false;
  return true;
}

bool
ElementSubdomainModifierBase::nodeIsNewlyActivated(dof_id_type node_id) const
{
  auto node = _mesh.nodePtr(node_id);

  int total_neighbor_elems;

  total_neighbor_elems = 0;
  for (auto neighbor_elem_id : _mesh.nodeToElemMap().at(node_id))
    if (_mesh.elemPtr(neighbor_elem_id)->subdomain_id() != _inactive_subdomain_ID
        /*exclude the element which has the inative subdomainID*/)
      total_neighbor_elems++;

  int reinitialized_neighbor_elems = 0;
  for (auto neighbor_elem_id : _mesh.nodeToElemMap().at(node_id))
    if (std::find(_global_reinitialized_elems.begin(),
                  _global_reinitialized_elems.end(),
                  neighbor_elem_id) != _global_reinitialized_elems.end())
      reinitialized_neighbor_elems++;

/// for debug
#ifndef NDEBUG
  auto pt_ptr = _mesh.nodePtr(node_id);
  auto pt = *pt_ptr;
  std::ofstream fout1("nodes_" + std::to_string(node_id) + ".txt", std::ios::app);
  if (fout1.is_open())
  {
    fout1 << pt(0) << ", " << pt(1) << "\n";
    fout1.close();
  }
  else
  {
    std::cerr << "Error: Unable to open nodes.txt for writing!" << std::endl;
  }
#endif

  if (reinitialized_neighbor_elems == total_neighbor_elems)
  {
#ifndef NDEBUG
    auto pt_ptr = _mesh.nodePtr(node_id);
    auto pt = *pt_ptr;
    if (_t > _dt)
    {
      std::ofstream fout("newly_activated_nodes_" + std::to_string(node_id) + ".txt",
                         std::ios::app);
      if (fout.is_open())
      {
        fout << pt(0) << ", " << pt(1) << "\n";
        fout.close();
      }
      else
      {
        std::cerr << "Error: Unable to open newly_activated_nodes.txt for writing!" << std::endl;
      }
    }
#endif
    return true;
  }
  // If all elements with the node are reinitialized, then the node is newly reinitialized.
  return false;
}

void
ElementSubdomainModifierBase::applyIC(bool displaced)
{
  switch (_ic_strategy)
  {
    case ICStrategyForNewlyActivated::IC_DEFAULT:
      // note: from IC -> current
      _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                       reinitializedBndNodeRange(displaced));
      break;

    case ICStrategyForNewlyActivated::IC_EXTRAPOLATE_SECOND_LAYER:
    case ICStrategyForNewlyActivated::IC_EXTRAPOLATE_FIRST_LAYER:
      if (_t >= _dt)
      {
        //         computeSecondNeighborInfo(_fe_problem.getNonlinearSystemBase(_sys.number()),
        //         displaced);
        // #ifndef NDEBUG
        //         verifySecondNeighborInfo();
        // #endif
        //         setCurrentSolutionsOnNewlyActivatedNodes(_fe_problem.getNonlinearSystemBase(_sys.number()));
        propagateICFromNeighborsLayerByLayer(_fe_problem.getNonlinearSystemBase(_sys.number()));
      }
      else
      {
        _fe_problem.projectInitialConditionOnCustomRange(reinitializedElemRange(displaced),
                                                         reinitializedBndNodeRange(displaced));
      }
      break;
    case ICStrategyForNewlyActivated::IC_POLYNOMIAL:
      applyIC_Polynomial(_fe_problem.getNonlinearSystemBase(_sys.number()));
      break;
    default:
      mooseError("Unknown initial condition strategy.");
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
    // std::cout << "elem_id = " << elem_id << "\n";
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

    dof_id_type bnode_id = bnode->id();
    Point bnode_pos = *bnode;
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

std::vector<const Node *>
ElementSubdomainModifierBase::firstLayerNeighbours(dof_id_type nid) const
{
  auto has_set_IC_node = [&](dof_id_type id)
  { return _node2IC_set.count(id) && _node2IC_set.at(id); };

  auto is_not_newly_activated = [&](dof_id_type id)
  {
    return std::find(_first_pass_global_activated_nodes.begin(),
                     _first_pass_global_activated_nodes.end(),
                     id) == _first_pass_global_activated_nodes.end();
  };

  std::vector<const Node *> neighbors;

  const auto & elem_ids = _mesh.nodeToElemMap().at(nid);
  for (auto eid : elem_ids)
  {
    const Elem * elem = _mesh.elemPtr(eid);
    const bool is_inactive = elem->subdomain_id() == _inactive_subdomain_ID;

    for (unsigned i = 0; i < elem->n_nodes(); ++i)
    {
      const Node * node_ptr = elem->node_ptr(i);
      const dof_id_type node_id = node_ptr->id();

      if (node_id == nid)
        continue;

      if (is_inactive)
      {
        if (has_set_IC_node(node_id))
        {
          std::cout << "id neighbor = " << node_id << "\n";
          std::cout << "node id = " << node_id << "\n";
          neighbors.push_back(node_ptr);
        }
      }
      else
      {
        if (is_not_newly_activated(node_id) || has_set_IC_node(node_id))
          neighbors.push_back(node_ptr);
      }
    }
  }

  return neighbors;
}

void
ElementSubdomainModifierBase::applyICForNodeList(SystemBase & sys,
                                                 const std::vector<dof_id_type> & nodes)
{
  NumericVector<Number> & vec = sys.solution();
  DofMap & dof_map = sys.dofMap();

  for (auto nid : nodes)
  {
    const auto it = _newlyactivated_node_to_second_neighbors.find(nid);
    if (it == _newlyactivated_node_to_second_neighbors.end())
      continue;
    const NeighborInfo & info = it->second;

    const Node * node = _mesh.nodePtr(nid);
    std::vector<dof_id_type> dofs;
    dof_map.dof_indices(node, dofs);

    std::vector<Real> num(dofs.size(), 0.0);
    Real denom = 0.0;

    for (size_t solution_dof = 0; solution_dof < info.distances.size(); ++solution_dof)
    {
      Real w = 1.0 / info.distances[solution_dof];
      denom += w;
      const auto & vals = info.solution_values[solution_dof];
      for (size_t j = 0; j < vals.size(); ++j)
        num[j] += w * vals[j];
    }
    for (size_t solution_dof = 0; solution_dof < dofs.size(); ++solution_dof)
    {
      std::cout << "solution_dof = " << solution_dof
                << ", dofs[solution_dof] = " << dofs[solution_dof]
                << ", num[solution_dof] = " << num[solution_dof] << ", denom = " << denom << "\n";
      vec.set(dofs[solution_dof], num[solution_dof] / denom);
    }
  }
  vec.close();
}

void
ElementSubdomainModifierBase::propagateICFromNeighborsLayerByLayer(SystemBase & sys)
{
  for (auto nid : _newactivated_nodes)
    _node2IC_set[nid] = false;

  int n_nodal_on_elem =
      _mesh.elemPtr(_mesh.nodeToElemMap().at(_final_global_activated_nodes[0])[0])->n_nodes();

  NumericVector<Number> & ghosted = sys.solution();
  ghosted.close();
  auto get_serial = [&]() -> NumericVector<Number> &
  {
    // sys.cleanSerializedSolution();
    // NumericVector<Number> & s = sys.serializedSolution();
    // ghosted.localize(s); // ghosted  (distributed)   ---MPI gather--->   s
    //                      // (NumericVector, SERIAL, full length)
    NumericVector<Number> & s = ghosted;
    return s;
  };

  NumericVector<Number> & current_solution = get_serial();

  DofMap & dof_map = sys.dofMap();

  std::queue<dof_id_type> nodes_q;

  // first layer neighbors of the newly activated nodes
  // we do not need to satisfy the search_neighbors_criteria here because this is enough
  for (auto new_id : _newactivated_nodes)
  {
    const auto neighbor_nodeList = firstLayerNeighbours(new_id);

    if (!neighbor_nodeList.empty())
      nodes_q.push(new_id);
  }

  int not_set_nodes_count = 0;

  // loop over the queue and prepare the queue for the next layer
  while (!nodes_q.empty())
  {
    size_t layer_size = nodes_q.size();
    std::vector<dof_id_type> this_layer_nodes;
    this_layer_nodes.reserve(layer_size);

    // loop over the nodes in the queue
    for (size_t layer_index = 0; layer_index < layer_size; ++layer_index)
    {
      auto nid = nodes_q.front();
      nodes_q.pop(); // erase the first element
      this_layer_nodes.push_back(nid);

      const Node * node_i = _mesh.nodePtr(nid);
      Point point_i = *node_i;

      NeighborInfo neighbor_info;
      for (auto nbr : firstLayerNeighbours(nid))
      {
        auto id = nbr->id();
        std::vector<dof_id_type> dofs;
        dof_map.dof_indices(nbr, dofs);
        std::vector<Real> vals;
        vals.reserve(dofs.size());
        for (auto dof : dofs)
        {
          std::cout << "current_solution(dof) = " << current_solution(dof) << "\n";

          Point point_nbr = *nbr;
          if (std::isnan(current_solution(dof)))
          {
            std::ofstream fout("is_nan_nodes.txt", std::ios::app);
            if (fout.is_open())
            {
              fout << point_i(0) << ", " << point_i(1) << ", " << point_nbr(0) << ", "
                   << point_nbr(1) << "\n";
              fout.close();
            }
            else
            {
              std::cerr << "Error: Unable to open is_nan_nodes.txt for writing!" << std::endl;
            }
          }

          vals.push_back(current_solution(dof));
        }
        Real dist = (*nbr - point_i).norm();
        std::cout << "dist = " << dist << "\n";
        neighbor_info.solution_values.push_back(std::move(vals));
        neighbor_info.distances.push_back(dist);
      }

      std::cout << "id go to map = " << nid << "\n";
      _newlyactivated_node_to_second_neighbors[nid] = std::move(neighbor_info) /*map*/;
      _node2IC_set[nid] = true;
    }

    // (a) Write back the values for this layer first
    applyICForNodeList(sys, this_layer_nodes);

    // (b) refresh serial so the next layer can read the values just written
    ghosted = sys.solution();
    ghosted.close();
    current_solution = get_serial(); // global solution

    // (c) unlock the next layer
    std::unordered_set<dof_id_type> nodes_in_queue;
    int search_neighbors_criteria = n_nodal_on_elem;

    while (search_neighbors_criteria >= 0)
    {
      not_set_nodes_count = 0;
      int nodes_set_this_round = 0;

      for (auto new_id : _newactivated_nodes)
      {
        if (_node2IC_set[new_id]) // skip if already set
          continue;

        const auto neighbor_nodeList = firstLayerNeighbours(new_id);

        if (neighbor_nodeList.size() >= search_neighbors_criteria)
        {
          nodes_q.push(new_id);
          nodes_set_this_round++;
        }

        not_set_nodes_count++;
      }

      if (nodes_set_this_round > 0)
        break; // if we have set some nodes, we can stop searching
      else
        search_neighbors_criteria--; // release the criteria
    }
  }

  if (nodes_q.empty() && not_set_nodes_count > 0)
  {
    std::cout << "There are some nodes not set.\n";
    for (auto it : _node2IC_set)
    {
      if (!it.second)
        std::cout << "id = " << it.first << "\n";
    }
  }
  else
  {
    std::cout << "All nodes are set.\n";
  }
}

void
ElementSubdomainModifierBase::applyIC_Polynomial(SystemBase & sys)
{

  std::vector<const Node *> neighbors;
  std::vector<dof_id_type> patch_elem_ids;

  auto is_not_newly_activated = [&](dof_id_type id)
  {
    return std::find(_first_pass_global_activated_nodes.begin(),
                     _first_pass_global_activated_nodes.end(),
                     id) == _first_pass_global_activated_nodes.end();
  };

  std::cout << "Number of newly activated nodes: " << _newactivated_nodes.size() << "\n";

  for (auto new_id : _newactivated_nodes)
  {
    // print node position
    Point node_pos = *_mesh.nodePtr(new_id);

    std::ofstream fout2("newly_activated_nodes.txt", std::ios::app);
    if (fout2.is_open())
    {
      fout2 << node_pos(0) << ", " << node_pos(1) << "\n";
      fout2.close();
    }
    else
    {
      std::cerr << "Error: Unable to open newly_activated_nodes.txt for writing!" << std::endl;
    }
  }

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

  // Pass 1: Traverse local active elements to find neighbors sharing nodes with reinit elements
  std::unordered_set<dof_id_type> patch_elem_set; // Prevent duplicates
  std::ofstream fout("boundary_nodes.txt", std::ios::out | std::ios::trunc);
  if (!fout.is_open())
    mooseError("Unable to open boundary_nodes.txt for writing!");

  for (const auto & elem : _mesh.getMesh().active_element_ptr_range())
  {
    const dof_id_type eid = elem->id();

    // (a) Skip if this is a reinitialized element
    if (reinit_elem_set.count(eid))
      continue;

    // (b) Skip if already added to patch
    if (patch_elem_set.count(eid))
      continue;

    // (c) Skip if element is not active
    if (elem->subdomain_id() == _inactive_subdomain_ID)
      continue;

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
      patch_elem_ids.push_back(eid);

      // Write all node coordinates to debug file
      for (unsigned n = 0; n < elem->n_nodes(); ++n)
      {
        const Node * node_ptr = elem->node_ptr(n);
        const Point & p = *node_ptr;
#if LIBMESH_DIM == 2
        fout << p(0) << ", " << p(1) << '\n';
#else
        fout << p(0) << ", " << p(1) << ", " << p(2) << '\n';
#endif
      }
    }
  }

  fout.close();

  std::cout << "Number of elements in the patch: " << patch_elem_set.size() << "\n";
  // for (const auto & id : patch_elem_ids)
  // {
  //   std::cout << "id = " << id << "\n";
  // }

  // Pass 2: setting IC for newly activated nodes

  NumericVector<Number> & vec = sys.solution();
  DofMap & dof_map = sys.dofMap();

  for (auto new_id : _newactivated_nodes)
  {
    const auto * node = _mesh.nodePtr(new_id);
    const auto & x = *node;

    // Recovery
    std::vector<Real> recovered_vals;
    int num_components = _npr_vec.size();
    std::cout << "num_components = " << num_components << "\n";
    for (unsigned comp = 0; comp < num_components; ++comp)
      recovered_vals.push_back(_npr_vec[comp]->nodalPatchRecovery(x, patch_elem_ids));

    std::vector<dof_id_type> dofs;
    dof_map.dof_indices(node, dofs);

    for (unsigned i = 0; i < dofs.size(); ++i)
    {
      std::cout << "recovered_vals[i] = " << recovered_vals[i] << "\n";
      vec.set(dofs[i], recovered_vals[i]);
    }
  }

  vec.close();
}

void
ElementSubdomainModifierBase::verifySecondNeighborInfo()
{
  mooseInfo("Verifying _newlyactivated_node_to_second_neighbors...");

  if (_newlyactivated_node_to_second_neighbors.empty())
  {
    mooseWarning("_newlyactivated_node_to_second_neighbors is empty.");
    return;
  }

  for (const auto & [bnode_id, neighbor_info] : _newlyactivated_node_to_second_neighbors)
  {
    mooseInfo("Boundary Node ID: {} has {} second-layer neighbors",
              bnode_id,
              neighbor_info.solution_values.size());

    if (neighbor_info.solution_values.size() != neighbor_info.distances.size())
    {
      mooseError("Mismatch: solution_values.size() != distances.size() for node ID {}", bnode_id);
    }

    for (std::size_t i = 0; i < neighbor_info.solution_values.size(); ++i)
    {
      mooseInfo("  Neighbor {}: solution = {}, distance = {}",
                i,
                neighbor_info.solution_values[i],
                neighbor_info.distances[i]);
    }
  }

  mooseInfo("Verification complete.");
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
ElementSubdomainModifierBase::gatherFirstPassActivatedNodesGlobally()
{
  std::vector<dof_id_type> local(_first_pass_local_activated_nodes.begin(),
                                 _first_pass_local_activated_nodes.end());
  std::vector<std::vector<dof_id_type>> gathered;

  _mesh.comm().allgather(local, gathered);

  std::unordered_set<dof_id_type> unique_nodes;

  for (const auto & vec : gathered)
    unique_nodes.insert(vec.begin(), vec.end());

  _first_pass_global_activated_nodes.assign(unique_nodes.begin(), unique_nodes.end());
}

void
ElementSubdomainModifierBase::gatherFinalActivatedNodesGlobally()
{
  std::vector<dof_id_type> local(_newactivated_nodes.begin(), _newactivated_nodes.end());
  std::vector<std::vector<dof_id_type>> gathered;

  _mesh.comm().allgather(local, gathered);

  _final_global_activated_nodes.clear();
  for (const auto & vec : gathered)
    _final_global_activated_nodes.insert(
        _final_global_activated_nodes.end(), vec.begin(), vec.end());
}

void
ElementSubdomainModifierBase::computeSetDifference()
{
  _final_global_activated_nodes_diff.clear();
  if (_final_global_activated_nodes == _first_pass_global_activated_nodes)
  {
    return;
  }

  std::vector<dof_id_type> nodes_sorted = _final_global_activated_nodes;
  std::vector<dof_id_type> temp_sorted = _first_pass_global_activated_nodes;

  std::sort(nodes_sorted.begin(), nodes_sorted.end());
  std::sort(temp_sorted.begin(), temp_sorted.end());

  std::vector<dof_id_type> difference;
  difference.reserve(nodes_sorted.size());

  std::set_difference(temp_sorted.begin(),
                      temp_sorted.end(),
                      nodes_sorted.begin(),
                      nodes_sorted.end(),
                      std::back_inserter(difference));

  // Store result
  _final_global_activated_nodes_diff = std::move(difference);

#ifndef NDEBUG
  // Report differences if any
  if (!_final_global_activated_nodes_diff.empty())
  {
    std::cout << "Rank " << _mesh.comm().rank() << " : Detected mismatch in activated nodes.\n"
              << "  Count in _final_global_activated_nodes      : "
              << _final_global_activated_nodes.size() << "\n"
              << "  Count in _first_pass_global_activated_nodes : "
              << _first_pass_global_activated_nodes.size() << "\n"
              << "  Difference count                         : "
              << _final_global_activated_nodes_diff.size() << "\n";

    // Optionally print IDs (can be removed if too verbose)
    std::cout << "  Nodes only in _final_global_activated_nodes:\n  ";
    for (const auto & id : _final_global_activated_nodes_diff)
      std::cout << id << " ";
    std::cout << "\n";
  }
#endif
}
