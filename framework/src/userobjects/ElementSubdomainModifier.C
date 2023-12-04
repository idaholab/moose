//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSubdomainModifier.h"
#include "DisplacedProblem.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"
#include "libmesh/dof_map.h"
#include "libmesh/remote_elem.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/petsc_vector.h"

InputParameters
ElementSubdomainModifier::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Modify element subdomain ID. This userobject only runs on the undisplaced mesh, and it will "
      "modify both the undisplaced and the displaced mesh.");
  params.addParam<BoundaryName>(
      "moving_boundary_name",
      "Boundary to modify when an element is moved. A boundary with the provided name will be "
      "created if not already exists on the mesh.");
  params.addParam<BoundaryName>("complement_moving_boundary_name",
                                "Boundary that associated with the unmoved domain when neighbor "
                                "element(s) is moved. A boundary with the provided name will be "
                                "created if not already exists on the mesh.");

  params.addParam<std::vector<VariableName>>(
      "initialize_variables",
      {},
      "Which variables to initialize when an element is moved into the active subdomains.");
  MooseEnum init_strategy("IC NEAREST CONSTANT", "IC");
  params.addParam<std::vector<MooseEnum>>(
      "initialization_strategy",
      {init_strategy},
      "Which strategy to use when initializing variables on newly activated elements and nodes. "
      "IC applies the variable initial condition to the newly activated dofs. NEAREST applies the "
      "old value from the nearest element/node to the newly activated dofs. CONSTANT initializes "
      "the newly activated dofs with a constant. This parameter should either have size of 1 or "
      "size equal to that of 'initialize_variables'. If the size of this parameter is 1, then the "
      "same strategy will be used for all variables listed in 'initialize_variables'.");
  params.addParam<std::vector<Real>>(
      "initialization_constant",
      "The constants to use when initialization_strategy = CONSTANT. This parameter should either "
      "have size of 1 or size equal to the number of variables with "
      "initialization_strategy==CONSTANT.");
  params.addParam<std::vector<SubdomainName>>(
      "active_subdomains",
      {},
      "The 'active' subdomains on which the simulation is performed, i.e. the PDE "
      "computational domain. If this parameter is left empty, then the entire mesh is treated as "
      "the activate subdomain.");

  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ElementSubdomainModifier::ElementSubdomainModifier(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _nl_sys(_fe_problem.getNonlinearSystemBase(systemNumber())),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _init_vars(getParam<std::vector<VariableName>>("initialize_variables")),
    _active_subdomains(
        _mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("active_subdomains"))),
    _moving_boundary_specified(isParamValid("moving_boundary_name")),
    _complement_moving_boundary_specified(isParamValid("complement_moving_boundary_name")),
    _moving_boundary_id(-1)
{
  for (auto var_name : getParam<std::vector<VariableName>>("initialize_variables"))
    if (!_nl_sys.hasVariable(var_name) && !_aux_sys.hasVariable(var_name))
      paramError("initialize_variables", "Variable ", var_name, " does not exist.");

  // The size of _init_strategy must be 1 or equal to the number of variables to initialize
  const auto init_strategy_in = getParam<std::vector<MooseEnum>>("initialization_strategy");
  if (_init_vars.size() == init_strategy_in.size())
    _init_strategy = init_strategy_in;
  else if (init_strategy_in.size() == 1)
    _init_strategy.resize(_init_vars.size(), init_strategy_in[0]);
  else
    paramError("initialization_strategy",
               "This parameter should either have size of 1 or size equal to "
               "that of 'initialize_variables'. If the size of this parameter is 1, then the "
               "same strategy will be used for all variables listed in 'initialize_variables'.");

  // The size of _init_constant must be 1 or equal to the number of variables to initialize (with
  // _init_strategy==CONSTANT)
  if (isParamValid("initialization_constant"))
  {
    const auto init_constant_in = getParam<std::vector<Real>>("initialization_constant");
    unsigned int count = 0;
    for (auto i : index_range(_init_vars))
      if (_init_strategy[i] == "CONSTANT")
      {
        if (init_constant_in.size() == 1)
          _init_constant[_init_vars[i]] = init_constant_in[0];
        else if (init_constant_in.size() > count)
          _init_constant[_init_vars[i]] = init_constant_in[count];
        else
          paramError("initialization_constant",
                     "This parameter should either have size of 1 or size equal to the number "
                     "variables with initialization_strategy == CONSTANT.");
        count++;
      }
  }

  // For steady-state simulation we can only apply variable IC
  if (!_fe_problem.isTransient() && !_init_vars.empty())
    for (auto & s : _init_strategy)
      if (s != "IC")
        paramError(
            "initial_condition_strategy",
            "IC is the only supported initialization strategy for steady-state simulations.");
}

void
ElementSubdomainModifier::initialSetup()
{
  if (_moving_boundary_specified)
  {
    _moving_boundary_name = getParam<BoundaryName>("moving_boundary_name");
    setMovingBoundaryName(_mesh);
    if (_displaced_problem)
      setMovingBoundaryName(_displaced_problem->mesh());
  }

  if (_complement_moving_boundary_specified)
  {
    _complement_moving_boundary_name = getParam<BoundaryName>("complement_moving_boundary_name");
    setComplementMovingBoundaryName(_mesh);
    if (_displaced_problem)
      setComplementMovingBoundaryName(_displaced_problem->mesh());
  }

  _nl_ndof = _nl_sys.system().n_dofs();
  _aux_ndof = _aux_sys.system().n_dofs();
}

void
ElementSubdomainModifier::serializeSolutionOld(dof_id_type ndof,
                                               SystemBase & sys,
                                               std::unique_ptr<NumericVector<Real>> & sol)
{
  if (!sol.get())
    sol = NumericVector<Real>::build(_communicator);

  sol->init(ndof, false, SERIAL);
  sys.solutionOld().localize(*sol);
}

void
ElementSubdomainModifier::timestepSetup()
{
  serializeSolutionOld(_nl_ndof, _nl_sys, _nl_sol_old);
  serializeSolutionOld(_aux_ndof, _aux_sys, _aux_sol_old);
}

void
ElementSubdomainModifier::setMovingBoundaryName(MooseMesh & mesh)
{
  // We only need one boundary to modify. Create a dummy vector just to use the API.
  const std::vector<BoundaryID> boundary_ids = mesh.getBoundaryIDs({{_moving_boundary_name}}, true);
  mooseAssert(boundary_ids.size() == 1, "Expect exactly one boundary ID.");
  _moving_boundary_id = boundary_ids[0];
  mesh.setBoundaryName(_moving_boundary_id, _moving_boundary_name);
  mesh.getMesh().get_boundary_info().sideset_name(_moving_boundary_id) = _moving_boundary_name;
  mesh.getMesh().get_boundary_info().nodeset_name(_moving_boundary_id) = _moving_boundary_name;
}

void
ElementSubdomainModifier::setComplementMovingBoundaryName(MooseMesh & mesh)
{
  // We only need one boundary to modify. Create a dummy vector just to use the API.
  const std::vector<BoundaryID> boundary_ids =
      mesh.getBoundaryIDs({{_complement_moving_boundary_name}}, true);
  mooseAssert(boundary_ids.size() == 1, "Expect exactly one boundary ID.");
  _complement_moving_boundary_id = boundary_ids[0];
  mesh.setBoundaryName(_complement_moving_boundary_id, _complement_moving_boundary_name);
  mesh.getMesh().get_boundary_info().sideset_name(_complement_moving_boundary_id) =
      _complement_moving_boundary_name;
  mesh.getMesh().get_boundary_info().nodeset_name(_complement_moving_boundary_id) =
      _complement_moving_boundary_name;
}

void
ElementSubdomainModifier::initialize()
{
  _moved_elems.clear();
  _moved_displaced_elems.clear();

  _activated_elems.clear();
  _activated_nodes.clear();
  _activated_bnd_nodes.clear();

  _cached_subdomain_assignments.clear();
  _moving_boundary_subdomains.clear();

  _elem_nearest_dofs.clear();
  _node_nearest_dofs.clear();
}

void
ElementSubdomainModifier::execute()
{
  // First, compute the desired subdomain ID for the current element.
  SubdomainID subdomain_id = computeSubdomainID();

  // If the current element's subdomain ID isn't what we want
  if (subdomain_id != std::numeric_limits<SubdomainID>::max() &&
      _current_elem->subdomain_id() != subdomain_id)
  {
    _moving_boundary_subdomains.insert(subdomain_id);
    _moving_boundary_subdomains.insert(_current_elem->subdomain_id());
    // Current element ID, used to index both the element on the displaced and undisplaced meshes.
    dof_id_type elem_id = _current_elem->id();
    // Change the element's subdomain
    Elem * elem = _mesh.elemPtr(elem_id);
    Elem * displaced_elem =
        _displaced_problem ? _displaced_problem->mesh().elemPtr(elem_id) : nullptr;

    // Save the activated elements so that we can later update/initialize the solution
    // If this element is not already on the active subdomain, this element is said to be newly
    // activated
    if (std::find(_active_subdomains.begin(),
                  _active_subdomains.end(),
                  _current_elem->subdomain_id()) != _active_subdomains.end())
      if (std::find(_activated_elems.begin(), _activated_elems.end(), elem) ==
          _activated_elems.end())
        _activated_elems.push_back(elem);

    // Save the activated nodes so that we can later update/initialize the solution
    // If none of the node neighbors is on the active subdomain, this node is said to be newly
    // activated
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
    {
      bool new_node = true;
      for (auto eid : _mesh.nodeToElemMap().at(elem->node_id(i)))
        if (std::find(_active_subdomains.begin(),
                      _active_subdomains.end(),
                      _mesh.elemPtr(eid)->subdomain_id()) != _active_subdomains.end())
        {
          new_node = false;
          break;
        }

      if (new_node &&
          std::find(_activated_nodes.begin(), _activated_nodes.end(), elem->node_ptr(i)) ==
              _activated_nodes.end())
        _activated_nodes.push_back(elem->node_ptr(i));
    }

    _cached_subdomain_assignments.emplace_back(elem, subdomain_id);
    _moved_elems.push_back(elem);
    if (displaced_elem)
    {
      _cached_subdomain_assignments.emplace_back(displaced_elem, subdomain_id);
      _moved_displaced_elems.push_back(displaced_elem);
    }

    // Change the parent's subdomain, if any
    // We do not save the parent info since they are inactive
    setAncestorsSubdomainIDs(subdomain_id, elem_id);
  }
}

void
ElementSubdomainModifier::threadJoin(const UserObject & in_uo)
{
  // Join the data from uo into _this_ object:
  const auto & uo = static_cast<const ElementSubdomainModifier &>(in_uo);

  _moved_elems.insert(_moved_elems.end(), uo._moved_elems.begin(), uo._moved_elems.end());

  _moved_displaced_elems.insert(_moved_displaced_elems.end(),
                                uo._moved_displaced_elems.begin(),
                                uo._moved_displaced_elems.end());

  _activated_elems.insert(
      _activated_elems.end(), uo._activated_elems.begin(), uo._activated_elems.end());
  _activated_nodes.insert(
      _activated_nodes.end(), uo._activated_nodes.begin(), uo._activated_nodes.end());
  _cached_subdomain_assignments.insert(_cached_subdomain_assignments.end(),
                                       uo._cached_subdomain_assignments.begin(),
                                       uo._cached_subdomain_assignments.end());
}

void
ElementSubdomainModifier::finalize()
{
  // If nothing need to change, just return.
  // This will skip all mesh changes, and so no adaptivity mesh files will need to be written.
  auto n_moved_elem = _cached_subdomain_assignments.size();
  gatherSum(n_moved_elem);
  if (n_moved_elem == 0)
    return;

  buildActivatedElemsRange();
  buildActivatedNodesRange();
  buildActivatedBndNodesRange();

  // Build nearest node and element maps *before* modifying the subdomain IDs
  for (auto i : index_range(_init_vars))
    if (_init_strategy[i] == "NEAREST")
    {
      if (_nl_sys.hasVariable(_init_vars[i]))
        findNearestDofs(_nl_sys, _init_vars[i]);
      else
        findNearestDofs(_aux_sys, _init_vars[i]);
    }

  // apply cached subdomain changes
  for (auto & [elem, subdomain_id] : _cached_subdomain_assignments)
    elem->subdomain_id() = subdomain_id;

  _ghost_sides_to_add.clear();
  _complement_ghost_sides_to_add.clear();
  _complement_ghost_sides_to_remove.clear();
  _complement_ghost_nodes_to_remove.clear();

  /*
    Synchronize ghost element subdomain ID
    Note: this needs to be done before updating boundary info because
    updating boundary requires the updated element subdomain ids
  */
  SyncSubdomainIds sync(_mesh.getMesh());
  Parallel::sync_dofobject_data_by_id(_mesh.getMesh().comm(),
                                      _mesh.getMesh().elements_begin(),
                                      _mesh.getMesh().elements_end(),
                                      sync);
  if (_moving_boundary_specified)
    updateMovingBoundaryInfo(_mesh, _moved_elems);
  if (!_moved_elems.empty() && _complement_moving_boundary_specified)
    updateComplementBoundaryInfo(_mesh, _moved_elems);

  // Similarly for the displaced mesh
  if (_displaced_problem)
  {
    SyncSubdomainIds sync_displaced(_displaced_problem->mesh().getMesh());
    Parallel::sync_dofobject_data_by_id(_displaced_problem->mesh().getMesh().comm(),
                                        _displaced_problem->mesh().getMesh().elements_begin(),
                                        _displaced_problem->mesh().getMesh().elements_end(),
                                        sync_displaced);
    if (_moving_boundary_specified)
      updateMovingBoundaryInfo(_displaced_problem->mesh(), _moved_displaced_elems);
    if (!_moved_displaced_elems.empty() && _complement_moving_boundary_specified)
      updateComplementBoundaryInfo(_displaced_problem->mesh(), _moved_displaced_elems);
  }

  // Synchronize boundary information after mesh update
  synchronizeBoundaryInfo(_mesh);

  if (_displaced_problem)
    synchronizeBoundaryInfo(_displaced_problem->mesh());

  // Reinit equation systems
  _fe_problem.meshChanged();

  // Apply initial condition for the newly activated dofs
  if (!_init_vars.empty())
  {
    // At step 0 we can _only_ apply IC
    if (_t_step == 0)
      _fe_problem.projectInitialConditionOnCustomRange(activatedElemsRange(),
                                                       activatedBndNodesRange());
    else
    {
      // Perform IC projection if _any_ variable requests IC as the initialization strategy
      for (auto & s : _init_strategy)
        if (s == "IC")
        {
          _fe_problem.projectInitialConditionOnCustomRange(activatedElemsRange(),
                                                           activatedBndNodesRange());
          break;
        }
      // Loop over each variable and initialize
      for (auto i : index_range(_init_vars))
      {
        if (_init_strategy[i] == "IC")
          continue; // no-op
        else if (_init_strategy[i] == "NEAREST")
        {
          if (_nl_sys.hasVariable(_init_vars[i]))
            setNearestSolutionForActivatedDofs(_nl_sys, _init_vars[i], *_nl_sol_old);
          else
            setNearestSolutionForActivatedDofs(_aux_sys, _init_vars[i], *_aux_sol_old);
        }
        else if (_init_strategy[i] == "CONSTANT")
        {
          if (_nl_sys.hasVariable(_init_vars[i]))
            setConstantForActivatedDofs(_nl_sys, _init_vars[i]);
          else
            setConstantForActivatedDofs(_aux_sys, _init_vars[i]);
        }
        else
          mooseError("Unsupported initialization strategy: ", _init_strategy[i]);
      }
    }
    // Set old and older solution on the initialized dofs
    _nl_sys.copyOldSolutions();
    _aux_sys.copyOldSolutions();
  }

  // Initialize stateful material properties for the newly activated elements
  _fe_problem.initElementStatefulProps(activatedElemsRange(), false);
}

void
ElementSubdomainModifier::meshChanged()
{
  // The number of dofs may have changed
  _nl_ndof = _nl_sys.system().n_dofs();
  _aux_ndof = _aux_sys.system().n_dofs();
}

void
ElementSubdomainModifier::updateMovingBoundaryInfo(MooseMesh & mesh,
                                                   const std::vector<const Elem *> & moved_elems)
{
  if (_moving_boundary_subdomains.size())
    mooseAssert(_moving_boundary_subdomains.size() == 2,
                "The number of moving subdomains should be two");
  /*
    There are a couple of steps to reconstruct the moving boundary.
    1) Retrieve all the active elements associated with the moving boundary
     in a previous step. These elements and their neighbors will serve as
     boundary element candidates for the current step.
    2) Remove all the elements from the moving boundary. That literately
     deletes the moving boundary from the database.
    3) Append moved elements to the boundary element candidates.
    4) Reconstruct the moving boundary using the boundary element candidates by
     computing the sides that are shared by two subdomains
    5) Delete the old nodeset.
    6) Reconstruct a new nodeset using the new sideset.
    7) Sync boundary information
  */

  BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();

  /*
   * Names can be deleted in the previous step if there is no side on this boundary
   */
  bnd_info.sideset_name(_moving_boundary_id) = _moving_boundary_name;
  bnd_info.nodeset_name(_moving_boundary_id) = _moving_boundary_name;

  auto & elem_side_bnd_ids = bnd_info.get_sideset_map();
  std::set<const Elem *> boundary_elem_candidates;
  std::vector<std::pair<const Elem *, unsigned int>> elem_sides_to_be_cleared;
  /*
   Check all the elements in the current moving boundary.
   If an element is active, add it to the boundary element list;
   otherwise, add active family members.
   At the same time, append all the elements, including the active and
   inactive, to a list being deleted later.
  */
  for (const auto & [elem, side_bnd] : elem_side_bnd_ids)
  {
    auto side = side_bnd.first;
    auto boundary_id = side_bnd.second;
    if (boundary_id == _moving_boundary_id)
    {
      if (elem->active())
        boundary_elem_candidates.insert(elem);
      else
      {
        auto top_parent = elem->top_parent();
        std::vector<const Elem *> active_family;
        top_parent->active_family_tree(active_family);
        for (auto felem : active_family)
          boundary_elem_candidates.insert((Elem *)felem);
      }
      elem_sides_to_be_cleared.emplace_back(elem, side);
    }
  }

  /* Delete the old moving boundary */
  for (auto & [elem, side] : elem_sides_to_be_cleared)
    bnd_info.remove_side(elem, side, _moving_boundary_id);

  /* Append moved elements to the boundary element candidate list */
  for (auto elem : moved_elems)
    boundary_elem_candidates.insert(elem);

  /* Go through the boundary element candidate list */
  for (auto elem : boundary_elem_candidates)
  {
    for (auto side : elem->side_index_range())
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      /*
       If elem's neighbor is active and has a different subdomain, we add the current side
       to the moving boundary
      */
      if (neighbor && neighbor->active() && neighbor != libMesh::remote_elem)
      {
        if (neighbor->subdomain_id() != elem->subdomain_id())
        {
          /*
           * If there is no new moved element, and then the elements from the original
           * moving boundary should be safe to use
           * If there are some new moved elements, then we need to check whether or not the
           * interface is between the moving subdomains
           */
          if (!_moving_boundary_subdomains.size() ||
              (_moving_boundary_subdomains.size() &&
               _moving_boundary_subdomains.find(neighbor->subdomain_id()) !=
                   _moving_boundary_subdomains.end() &&
               _moving_boundary_subdomains.find(elem->subdomain_id()) !=
                   _moving_boundary_subdomains.end()))
          {
            bnd_info.add_side(elem, side, _moving_boundary_id);
          }
        }
      }
      /*
       If elem's neighbor is not active, we need to check family members of the neighbor.
       In this case, the neighbor's children are on the current side and the children are
       "smaller" than the current element. We so add the neighboring children to the moving
       boundary list. Assigning "smaller" elements to the moving_boundary list is necessary in
       order to correctly represent the moving boundary.
      */
      else if (neighbor && !neighbor->active() && neighbor != libMesh::remote_elem)
      {
        std::vector<const Elem *> active_family;
        auto top_parent = neighbor->top_parent();
        top_parent->active_family_tree_by_neighbor(active_family, elem);
        for (auto felem : active_family)
        {
          if (felem->subdomain_id() != elem->subdomain_id())
          {
            auto cside = felem->which_neighbor_am_i(elem);
            // Add all the sides to the boundary first and remove excessive sides later
            bnd_info.add_side(felem, cside, _moving_boundary_id);
            if (felem->processor_id() != this->processor_id())
              _ghost_sides_to_add[felem->processor_id()].emplace_back(felem->id(), cside);
          }
        }
      }
    }
  }

  /* Delete the corresponding nodeset as well */
  auto & nodeset_map = bnd_info.get_nodeset_map();
  std::vector<const Node *> nodes_elem_sides_to_be_cleared;
  for (const auto & pair : nodeset_map)
    if (pair.second == _moving_boundary_id)
      nodes_elem_sides_to_be_cleared.push_back(pair.first);

  for (const auto node : nodes_elem_sides_to_be_cleared)
    bnd_info.remove_node(node, _moving_boundary_id);

  /* Reconstruct a new nodeset from the updated sideset */
  std::set<const Node *> boundary_nodes;
  for (const auto & [elem, side_bnd] : elem_side_bnd_ids)
  {
    auto side = side_bnd.first;
    auto boundary_id = side_bnd.second;
    if (boundary_id == _moving_boundary_id)
    {
      auto nodes = elem->nodes_on_side(side);
      for (auto node : nodes)
        boundary_nodes.insert(&(elem->node_ref(node)));
    }
  }

  for (const auto node : boundary_nodes)
    bnd_info.add_node(node, _moving_boundary_id);
}

void
ElementSubdomainModifier::updateComplementBoundaryInfo(
    MooseMesh & mesh, const std::vector<const Elem *> & moved_elems)
{
  BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();

  // The logic below updates the side set and the node set associated with the moving boundary.
  std::set<dof_id_type> added_nodes, removed_nodes;
  for (auto elem : moved_elems)
  {
    // First loop over all the sides of the element
    for (auto side : elem->side_index_range())
    {
      const Elem * neighbor = elem->neighbor_ptr(side);
      bnd_info.remove_side(elem, side, _complement_moving_boundary_id);
      if (neighbor && neighbor != libMesh::remote_elem)
      {
        // If the neighbor has a different subdomain ID, then this side should be added to
        // the moving boundary
        if (neighbor->subdomain_id() != elem->subdomain_id())
        {

          unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);
          bnd_info.add_side(neighbor, neighbor_side, _complement_moving_boundary_id);
          if (neighbor->processor_id() != this->processor_id())
          {
            _complement_ghost_sides_to_add[neighbor->processor_id()].emplace_back(neighbor->id(),
                                                                                  neighbor_side);
          }
        }
        // Otherwise remove this side and the neighbor side from the boundary.
        else
        {
          // this will destroy any overlapping third side sets!
          bnd_info.remove_side(elem, side, _complement_moving_boundary_id);
          unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);
          bnd_info.remove_side(neighbor, neighbor_side);
          if (neighbor->processor_id() != this->processor_id())
            _complement_ghost_sides_to_remove[neighbor->processor_id()].emplace_back(neighbor->id(),
                                                                                     neighbor_side);
        }
      }
    }

    // Then loop over all the nodes of the element
    for (auto node : elem->node_index_range())
    {
      // Find the point neighbors
      std::set<const Elem *> neighbor_set;
      elem->find_point_neighbors(elem->node_ref(node), neighbor_set);
      for (auto neighbor : neighbor_set)
        if (neighbor != libMesh::remote_elem)
        {
          // If the neighbor has a different subdomain ID, then this node should be added to
          // the moving boundary
          if (neighbor->subdomain_id() != elem->subdomain_id())
            added_nodes.insert(elem->node_id(node));
          // Otherwise remove this node from the boundary.
          else
          {
            removed_nodes.insert(elem->node_id(node));
            if (neighbor->processor_id() != this->processor_id())
              _complement_ghost_nodes_to_remove[neighbor->processor_id()].push_back(
                  elem->node_id(node));
          }
        }
    }
  }

  // make sure to remove only nodes that are not in the add set
  std::set<dof_id_type> nodes_to_remove;
  std::set_difference(removed_nodes.begin(),
                      removed_nodes.end(),
                      added_nodes.begin(),
                      added_nodes.end(),
                      std::inserter(nodes_to_remove, nodes_to_remove.end()));
  for (auto node_id : nodes_to_remove)
  {
    if (_complement_moving_boundary_specified)
      mesh.getMesh().get_boundary_info().remove_node(mesh.nodePtr(node_id),
                                                     _complement_moving_boundary_id);
  }
}

void
ElementSubdomainModifier::synchronizeBoundaryInfo(MooseMesh & mesh)
{
  pushBoundarySideInfo(mesh);
  pushBoundaryNodeInfo(mesh);
  mesh.getMesh().get_boundary_info().parallel_sync_side_ids();
  mesh.getMesh().get_boundary_info().parallel_sync_node_ids();
  mesh.update();
}

void
ElementSubdomainModifier::pushBoundarySideInfo(MooseMesh & moose_mesh)
{
  auto & mesh = moose_mesh.getMesh();
  auto elem_remove_functor =
      [&mesh, this](processor_id_type,
                    const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem)
  {
    // remove the side
    for (const auto & pr : received_elem)
      mesh.get_boundary_info().remove_side(
          mesh.elem_ptr(pr.first), pr.second, _complement_moving_boundary_id);
  };

  // We create a tempalte functor to add with custom boundary IDs
  auto elem_add_functor_with_boundary_id =
      [&mesh](const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem,
              const BoundaryID boundary_id)
  {
    // add the side
    for (const auto & pr : received_elem)
      mesh.get_boundary_info().add_side(mesh.elem_ptr(pr.first), pr.second, boundary_id);
  };

  // Then we use it with the regular and the complement boundary id
  auto complement_elem_add_functor =
      [this, elem_add_functor_with_boundary_id](
          processor_id_type,
          const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem)
  { elem_add_functor_with_boundary_id(received_elem, _complement_moving_boundary_id); };

  auto elem_add_functor =
      [this, elem_add_functor_with_boundary_id](
          processor_id_type,
          const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem)
  { elem_add_functor_with_boundary_id(received_elem, _moving_boundary_id); };

  // Push/pull the ghost cell sides for the regular and complement boundaries
  Parallel::push_parallel_vector_data(
      mesh.get_boundary_info().comm(), _ghost_sides_to_add, elem_add_functor);
  Parallel::push_parallel_vector_data(
      mesh.get_boundary_info().comm(), _complement_ghost_sides_to_remove, elem_remove_functor);
  if (_complement_moving_boundary_specified)
    Parallel::push_parallel_vector_data(mesh.get_boundary_info().comm(),
                                        _complement_ghost_sides_to_add,
                                        complement_elem_add_functor);
}

void
ElementSubdomainModifier::pushBoundaryNodeInfo(MooseMesh & moose_mesh)
{
  auto & mesh = moose_mesh.getMesh();
  auto node_remove_functor =
      [&mesh, this](processor_id_type, const std::vector<dof_id_type> & received_nodes)
  {
    for (const auto & pr : received_nodes)
    {
      if (_moving_boundary_specified)
        mesh.get_boundary_info().remove_node(mesh.node_ptr(pr), _moving_boundary_id);
      if (_complement_moving_boundary_specified)
        mesh.get_boundary_info().remove_node(mesh.node_ptr(pr), _complement_moving_boundary_id);
    }
  };

  Parallel::push_parallel_vector_data(
      mesh.get_boundary_info().comm(), _complement_ghost_nodes_to_remove, node_remove_functor);
}

void
ElementSubdomainModifier::buildActivatedElemsRange()
{
  // Clear the object first
  _activated_elems_range.reset();

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(_activated_elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + _activated_elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  _activated_elems_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);
}

void
ElementSubdomainModifier::buildActivatedNodesRange()
{
  // Clear the object first
  _activated_nodes_range.reset();

  // Make some fake element iterators defining this vector of elements
  Node * const * node_itr_begin = const_cast<Node * const *>(_activated_nodes.data());
  Node * const * node_itr_end = node_itr_begin + _activated_nodes.size();

  const auto nodes_begin = MeshBase::const_node_iterator(
      node_itr_begin, node_itr_end, Predicates::NotNull<Node * const *>());
  const auto nodes_end = MeshBase::const_node_iterator(
      node_itr_end, node_itr_end, Predicates::NotNull<Node * const *>());

  _activated_nodes_range = std::make_unique<ConstNodeRange>(nodes_begin, nodes_end);
}

void
ElementSubdomainModifier::buildActivatedBndNodesRange()
{
  // Clear the object first:
  _activated_bnd_nodes_range.reset();

  // Find activated boundary node
  for (auto & bnd_node : *_mesh.getBoundaryNodeRange())
    if (std::find(_activated_nodes.begin(), _activated_nodes.end(), bnd_node->_node) !=
        _activated_nodes.end())
      _activated_bnd_nodes.push_back(bnd_node);

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(_activated_bnd_nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + _activated_bnd_nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  _activated_bnd_nodes_range = std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);
}

void
ElementSubdomainModifier::findNearestDofs(SystemBase & sys, const VariableName & var_name)
{
  std::map<dof_id_type, Real> nearest_distance;

  // Get the variable
  auto & var = sys.getVariable(_tid, var_name);

  // Get the variable number
  auto var_num = var.number();

  // Get dofmap
  const auto & dofmap = sys.dofMap();

  for (auto elem : _mesh.getMesh().active_element_ptr_range())
  {
    if (!_active_subdomains.empty() &&
        std::find(_active_subdomains.begin(), _active_subdomains.end(), elem->subdomain_id()) ==
            _active_subdomains.end())
      continue;

    // The variable could have nodal dofs
    for (const auto & activated_node : activatedNodesRange())
      for (const auto & node : elem->node_ref_range())
      {
        // Skip if this node doesn't store dofs of this variable
        if (node.n_comp(sys.number(), var_num) == 0)
          continue;

        if (_node_nearest_dofs[var_num].count(activated_node->id()))
        {
          Real dist = (node - *activated_node).norm();
          if (dist < nearest_distance[activated_node->id()])
          {
            nearest_distance[activated_node->id()] = dist;
            dofmap.dof_indices(&node, _node_nearest_dofs[var_num][activated_node->id()], var_num);
          }
        }
        else
        {
          nearest_distance[activated_node->id()] = std::numeric_limits<Real>::max();
          dofmap.dof_indices(&node, _node_nearest_dofs[var_num][activated_node->id()], var_num);
        }
      }
    // or elemental dofs
    for (const auto & activated_elem : activatedElemsRange())
    {
      // Skip if this element doesn't store dofs of this variable
      if (elem->n_comp(sys.number(), var_num) == 0)
        continue;

      if (_elem_nearest_dofs[var_num].count(activated_elem->id()))
      {
        Real dist = (elem->centroid() - activated_elem->centroid()).norm();
        if (dist < nearest_distance[activated_elem->id()])
        {
          nearest_distance[activated_elem->id()] = dist;
          dofmap.dof_indices(elem, _elem_nearest_dofs[var_num][activated_elem->id()], var_num);
        }
      }
      else
      {
        nearest_distance[activated_elem->id()] = std::numeric_limits<Real>::max();
        dofmap.dof_indices(elem, _elem_nearest_dofs[var_num][activated_elem->id()], var_num);
      }
    }
  }
}

void
ElementSubdomainModifier::setNearestSolutionForActivatedDofs(SystemBase & sys,
                                                             const VariableName & var_name,
                                                             NumericVector<Real> & old_solution)
{
  // Get the variable
  auto & var = sys.getVariable(_tid, var_name);

  // Get the variable number
  auto var_num = var.number();

  auto & current_solution = sys.solution();
  DofMap & dof_map = sys.dofMap();

  std::vector<dof_id_type> dofs;
  std::vector<dof_id_type> nearest_dofs;

  if (_node_nearest_dofs.count(var_num))
  {
    for (auto & [activated_node_id, nearest_node_dofs] : _node_nearest_dofs[var_num])
    {
      auto activated_node = _mesh.nodePtr(activated_node_id);
      // Dofs on the activated node
      std::vector<dof_id_type> node_dofs;
      dof_map.dof_indices(activated_node, node_dofs, var_num);
      dofs.insert(dofs.end(), node_dofs.begin(), node_dofs.end());

      // Dofs on the nearest node
      nearest_dofs.insert(nearest_dofs.end(), nearest_node_dofs.begin(), nearest_node_dofs.end());
    }
  }

  if (_elem_nearest_dofs.count(var_num))
  {
    for (auto & [activated_elem_id, nearest_elem_dofs] : _elem_nearest_dofs[var_num])
    {
      auto activated_elem = _mesh.elemPtr(activated_elem_id);
      // Dofs on the activated element
      std::vector<dof_id_type> elem_dofs;
      dof_map.dof_indices(activated_elem, elem_dofs, var_num);
      dofs.insert(dofs.end(), elem_dofs.begin(), elem_dofs.end());

      // Dofs on the nearest element
      nearest_dofs.insert(nearest_dofs.end(), nearest_elem_dofs.begin(), nearest_elem_dofs.end());
    }
  }

  for (auto i : index_range(dofs))
    current_solution.set(dofs[i], old_solution(nearest_dofs[i]));

  current_solution.close();
  old_solution.close();
}

void
ElementSubdomainModifier::setConstantForActivatedDofs(SystemBase & sys,
                                                      const VariableName & var_name)
{
  // Get the variable
  auto & var = sys.getVariable(_tid, var_name);

  // Get the variable number
  auto var_num = var.number();

  auto & current_solution = sys.solution();
  DofMap & dof_map = sys.dofMap();

  std::vector<dof_id_type> dofs;

  for (auto activated_node : activatedNodesRange())
  {
    std::vector<dof_id_type> node_dofs;
    dof_map.dof_indices(activated_node, node_dofs, var_num);
    dofs.insert(dofs.end(), node_dofs.begin(), node_dofs.end());
  }

  for (auto activated_elem : activatedElemsRange())
  {
    std::vector<dof_id_type> elem_dofs;
    dof_map.dof_indices(activated_elem, elem_dofs, var_num);
    dofs.insert(dofs.end(), elem_dofs.begin(), elem_dofs.end());
  }

  for (auto i : index_range(dofs))
    current_solution.set(dofs[i], _init_constant[var_name]);

  current_solution.close();
}

void
ElementSubdomainModifier::setAncestorsSubdomainIDs(const SubdomainID & subdomain_id,
                                                   const dof_id_type & elem_id)
{
  auto curr_elem = _mesh.elemPtr(elem_id);

  unsigned int lv = curr_elem->level();

  for (unsigned int i = lv; i > 0; --i)
  {
    // Change the parent's subdomain, if any
    curr_elem = curr_elem->parent();
    dof_id_type id = curr_elem->id();
    auto elem = _mesh.elemPtr(id);
    elem->subdomain_id() = subdomain_id;

    // displaced parent element
    auto displaced_elem = _displaced_problem ? _displaced_problem->mesh().elemPtr(id) : nullptr;
    if (displaced_elem)
      displaced_elem->subdomain_id() = subdomain_id;
  }
}
