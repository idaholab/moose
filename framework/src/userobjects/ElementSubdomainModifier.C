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

  params.addParam<std::vector<BoundaryName>>(
      "moving_boundaries",
      {},
      "Moving boundaries between subdomains. These boundaries (both sidesets and nodesets) will be "
      "updated as elements change their subdomain. The corresponding subdomains of each moving "
      "boundary shall be specified using the parameter 'moving_boundary_subdomain_pairs'. A "
      "boundary will be created on the mesh if it does not already exist.");
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
      "active_subdomains",
      {},
      "The variables specified by the parameter 'initialize_variables' are initialized to their "
      "initial conditions when elements are moved from 'inactive' subdomains to 'active' "
      "subdomains, or from an 'active' subdomain to a different 'active' subdomain. The 'active' "
      "subdomains are usually where the simulation is performed, i.e. the PDE's computational "
      "domain. If this parameter is left empty, then the entire mesh is treated as the active "
      "subdomain.");
  params.addParam<std::vector<VariableName>>("initialize_variables",
                                             {},
                                             "Variables to initialize when an element is moved "
                                             "from inactive subdomains to the active subdomains.");
  params.addRequiredParam<dof_id_type>("debug_element_id", "Debug element id");

  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ElementSubdomainModifier::ElementSubdomainModifier(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _displaced_mesh(_displaced_problem ? &_displaced_problem->mesh() : nullptr),
    _nl_sys(_fe_problem.getNonlinearSystemBase(systemNumber())),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _active_subdomains(
        _mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("active_subdomains"))),
    _init_vars(getParam<std::vector<VariableName>>("initialize_variables")),
    _dbg_elem_id(getParam<dof_id_type>("debug_element_id"))
{
}

void
ElementSubdomainModifier::initialSetup()
{
  for (auto var_name : _init_vars)
    if (!_nl_sys.hasVariable(var_name) && !_aux_sys.hasVariable(var_name))
      paramError("initialize_variables", "Variable ", var_name, " does not exist.");

  initializeMovingBoundaries();
}

void
ElementSubdomainModifier::initializeMovingBoundaries()
{
  const auto bnd_names = getParam<std::vector<BoundaryName>>("moving_boundaries");
  const auto bnd_ids = _mesh.getBoundaryIDs(bnd_names, true);
  const auto bnd_subdomains =
      getParam<std::vector<std::vector<SubdomainName>>>("moving_boundary_subdomain_pairs");

  if (bnd_names.size() != bnd_subdomains.size())
    paramError("moving_boundary_subdomain_pairs",
               "Each moving boundary must correspond to a pair of subdomains. ",
               bnd_names.size(),
               " boundaries are specified by the parameter 'moving_boundaries', while ",
               bnd_subdomains.size(),
               " subdomain paris are provided.");

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
ElementSubdomainModifier::createMovingBoundaries(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();
  for (const auto & [bnd_id, bnd_name] : _moving_boundary_names)
  {
    bnd_info.sideset_name(bnd_id) = bnd_name;
    bnd_info.nodeset_name(bnd_id) = bnd_name;
  }
}

void
ElementSubdomainModifier::initialize()
{
  // Clear moved elements from last execution
  _moved_elems.clear();
  _activated_elems.clear();
  _activated_nodes.clear();

  // Clear moving boundary changes from last execution
  _add_element_sides.clear();
  _add_neighbor_sides.clear();
  _remove_element_sides.clear();
  _remove_neighbor_sides.clear();

  // Create moving boundaries on the undisplaced and displaced meshes
  //
  // Note: We do this at initialize() because previous execution might have removed the sidesets and
  // nodesets. Most of the moving boundary algorithm below assumes that the moving sidesets and
  // nodesets already exist on the mesh.
  createMovingBoundaries(_mesh);
  if (_displaced_mesh)
    createMovingBoundaries(*_displaced_mesh);
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
    // Current element ID, used to index both the element on the displaced and undisplaced meshes.
    dof_id_type elem_id = _current_elem->id();

    // Cache element subdomain assignment
    _moved_elems[elem_id] = {_current_elem->subdomain_id(), subdomain_id};

    // Save the activated elements and nodes so that we can later update/initialize the solution.
    //
    // Note that we only work with the undisplaced mesh for solution initialization. The solution
    // vectors are then synced to the displaced mesh. This could be generalized in the future to
    // provide the option to evaluate solution initialization on the displaced mesh as well.
    if (subdomainIsActive(subdomain_id))
    {
      _activated_elems.insert(elem_id);
      for (unsigned int i = 0; i < _current_elem->n_nodes(); ++i)
        if (nodeIsNewlyActivated(_current_elem->node_id(i)))
          _activated_nodes.insert(_current_elem->node_id(i));
    }

    // Change the parent's subdomain, if any
    // We do not save the parent info since they are inactive
    setAncestorsSubdomainIDs(subdomain_id, elem_id);
  }
}

bool
ElementSubdomainModifier::subdomainIsActive(SubdomainID id) const
{
  // If no active subdomain is specified, the entire mesh is treated as active
  if (_active_subdomains.empty())
    return true;

  return std::find(_active_subdomains.begin(), _active_subdomains.end(), id) !=
         _active_subdomains.end();
}

bool
ElementSubdomainModifier::nodeIsNewlyActivated(dof_id_type node_id) const
{
  // If any of the node neighbors is already active, then the node is NOT newly activated.
  for (auto neighbor_elem_id : _mesh.nodeToElemMap().at(node_id))
    if (subdomainIsActive(_mesh.elemPtr(neighbor_elem_id)->subdomain_id()))
      return false;
  return true;
}

void
ElementSubdomainModifier::threadJoin(const UserObject & in_uo)
{
  // Join the data from uo into _this_ object:
  const auto & uo = static_cast<const ElementSubdomainModifier &>(in_uo);

  _moved_elems.insert(uo._moved_elems.begin(), uo._moved_elems.end());
  _activated_elems.insert(uo._activated_elems.begin(), uo._activated_elems.end());
  _activated_nodes.insert(uo._activated_nodes.begin(), uo._activated_nodes.end());
}

void
ElementSubdomainModifier::finalize()
{
  // If nothing need to change, just return.
  // This will skip all mesh changes, and so no adaptivity mesh files will be written.
  auto n_moved_elem = _moved_elems.size();
  gatherSum(n_moved_elem);
  if (n_moved_elem == 0)
    return;

  // Apply cached subdomain changes
  applySubdomainChanges(_mesh);
  if (_displaced_mesh)
    applySubdomainChanges(*_displaced_mesh);

  // Update moving boundaries
  gatherMovingBoundaryChanges();
  applyMovingBoundaryChanges(_mesh);
  if (_displaced_mesh)
    applyMovingBoundaryChanges(*_displaced_mesh);

  // Reinit equation systems
  _fe_problem.meshChanged();

  // Initialize solution and stateful material properties
  applyIC(/*displaced=*/false);
  initElementStatefulProps(/*displaced=*/false);

  if (_displaced_mesh)
  {
    applyIC(/*displaced=*/true);
    initElementStatefulProps(/*displaced=*/true);
  }
}

void
ElementSubdomainModifier::applySubdomainChanges(MooseMesh & mesh)
{
  for (const auto & [elem_id, subdomain] : _moved_elems)
  {
    const auto & [from, to] = subdomain;
    mooseAssert(mesh.elemPtr(elem_id)->subdomain_id() == from,
                "Inconsistent element subdomain ID.");
    mesh.elemPtr(elem_id)->subdomain_id() = to;
  }

  // Synchronize ghost element subdomain changes
  SyncSubdomainIds sync(mesh.getMesh());
  Parallel::sync_dofobject_data_by_id(
      mesh.getMesh().comm(), mesh.getMesh().elements_begin(), mesh.getMesh().elements_end(), sync);
}

void
ElementSubdomainModifier::applyIC(bool displaced)
{
  if (!_init_vars.empty())
    _fe_problem.projectInitialConditionOnCustomRange(activatedElemRange(displaced),
                                                     activatedBndNodeRange(displaced));
}

void
ElementSubdomainModifier::initElementStatefulProps(bool displaced)
{
  _fe_problem.initElementStatefulProps(activatedElemRange(displaced), /*threaded=*/true);
}

void
ElementSubdomainModifier::meshChanged()
{
  // Clear cached ranges
  _activated_elem_range.reset();
  _activated_displaced_elem_range.reset();
  _activated_bnd_node_range.reset();
  _activated_displaced_bnd_node_range.reset();

  removeInactiveMovingBoundary(_mesh);
  if (_displaced_mesh)
    removeInactiveMovingBoundary(*_displaced_mesh);
}

void
ElementSubdomainModifier::removeInactiveMovingBoundary(MooseMesh & mesh)
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
      if (elem->id() == _dbg_elem_id)
        std::cout << "**removing side** elem " << elem->id() << " side " << side << " bnd " << bnd
                  << std::endl;
      bnd_info.remove_side(elem, side, bnd);

      std::vector<const Elem *> elem_family;
      elem->active_family_tree_by_side(elem_family, side);
      for (auto felem : elem_family)
        bnd_info.add_side(felem, side, bnd);
    }
  }

  bnd_info.parallel_sync_side_ids();
  bnd_info.parallel_sync_node_ids();
  mesh.update();
}

void
ElementSubdomainModifier::gatherMovingBoundaryChanges()
{
  const auto & sidesets = _mesh.getMesh().get_boundary_info().get_sideset_map();

  for (const auto & [elem_id, subdomain_assignment] : _moved_elems)
  {
    if (elem_id == _dbg_elem_id)
      std::cout << "---------------------------------\n";

    auto elem = _mesh.elemPtr(elem_id);
    const auto & [from_subdomain, to_subdomain] = subdomain_assignment;

    // The existing moving boundaries on the element side should be removed
    for (auto itr = sidesets.lower_bound(elem); itr != sidesets.upper_bound(elem); itr++)
      if (_moving_boundary_names.count(itr->second.second))
      {
        if (elem->id() == _dbg_elem_id)
          std::cout << "    removing side: elem " << elem->id() << " side " << itr->second.first
                    << " bnd " << itr->second.second << std::endl;
        _remove_element_sides[elem->id()].emplace(itr->second.first, itr->second.second);
      }

    for (auto side : elem->side_index_range())
    {
      auto neigh = elem->neighbor_ptr(side);

      // Don't mess with remote element neighbor
      if (neigh && neigh == libMesh::remote_elem)
        continue;
      // If neighbor doesn't exist
      else if (!neigh)
        gatherMovingBoundaryChangesHelper(elem, side, nullptr, 0, to_subdomain);
      // If neighbor exists
      else
      {
        auto neigh_side = neigh->which_neighbor_am_i(elem);

        if (elem_id == _dbg_elem_id)
          std::cout << "side " << side << " neighbor " << neigh->id() << std::endl;

        if (neigh->active())
        {
          if (elem_id == _dbg_elem_id)
            std::cout << "  active" << std::endl;
          gatherMovingBoundaryChangesHelper(elem, side, neigh, neigh_side, to_subdomain);
        }
        else
        {
          if (elem_id == _dbg_elem_id)
            std::cout << "  inactive" << std::endl;
          std::vector<const Elem *> active_neighs;
          neigh->top_parent()->active_family_tree_by_neighbor(active_neighs, elem);
          for (auto active_neigh : active_neighs)
          {
            if (elem_id == _dbg_elem_id || active_neigh->id() == _dbg_elem_id)
              std::cout << "    elem " << elem_id << " neighbor child " << active_neigh->id()
                        << std::endl;
            gatherMovingBoundaryChangesHelper(elem, side, active_neigh, neigh_side, to_subdomain);
          }
        }
      }
    }
  }
}

void
ElementSubdomainModifier::gatherMovingBoundaryChangesHelper(const Elem * elem,
                                                            unsigned short side,
                                                            const Elem * neigh,
                                                            unsigned short neigh_side,
                                                            SubdomainID to_subdomain)
{
  const auto & sidesets = _mesh.getMesh().get_boundary_info().get_sideset_map();

  // Detect element side change
  SubdomainPair subdomain_pair = {to_subdomain,
                                  neigh ? neigh->subdomain_id() : Moose::INVALID_BLOCK_ID};
  if (_moving_boundaries.count(subdomain_pair))
    _add_element_sides[elem->id()].emplace(side, _moving_boundaries.at(subdomain_pair));

  // Detect neighbor side change (by reversing the subdomain pair)
  if (neigh)
  {
    subdomain_pair = {subdomain_pair.second, subdomain_pair.first};
    if (_moving_boundaries.count(subdomain_pair))
      _add_neighbor_sides[neigh->id()].emplace(neigh_side, _moving_boundaries.at(subdomain_pair));

    // The existing moving boundaries on the neighbor side should be removed
    for (auto itr = sidesets.lower_bound(neigh); itr != sidesets.upper_bound(neigh); itr++)
      if (itr->second.first == neigh_side && _moving_boundary_names.count(itr->second.second))
      {
        if (elem->id() == _dbg_elem_id || neigh->id() == _dbg_elem_id)
          std::cout << "      removing neighbor side " << neigh_side << std::endl;
        _remove_neighbor_sides[neigh->id()].emplace(itr->second.first, itr->second.second);
      }
  }
}

void
ElementSubdomainModifier::applyMovingBoundaryChanges(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();

  std::unordered_map<processor_id_type,
                     std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>>>
      add_ghost_sides, remove_ghost_sides;

  // Remove element sides from moving boundaries
  for (const auto & [elem_id, sides] : _remove_element_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
    {
      if (elem->id() == _dbg_elem_id)
        std::cout << "**removing element side** elem " << elem->id() << " side " << side << " bnd "
                  << bnd << std::endl;
      bnd_info.remove_side(elem, side, bnd);
    }
  }

  // Remove neighbor sides from moving boundaries
  for (const auto & [elem_id, sides] : _remove_neighbor_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
    {
      if (elem->id() == _dbg_elem_id)
        std::cout << "**removing neighbor side** elem " << elem->id() << " side " << side << " bnd "
                  << bnd << std::endl;
      bnd_info.remove_side(elem, side, bnd);
      if (elem->processor_id() != processor_id())
        remove_ghost_sides[elem->processor_id()].push_back({elem_id, side, bnd});
    }
  }

  // Add element sides to moving boundaries
  for (const auto & [elem_id, sides] : _add_element_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
      bnd_info.add_side(elem, side, bnd);
  }

  // Add neighbor sides to moving boundaries
  for (const auto & [elem_id, sides] : _add_neighbor_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & [side, bnd] : sides)
    {
      bnd_info.add_side(elem, side, bnd);
      if (elem->processor_id() != processor_id())
        add_ghost_sides[elem->processor_id()].push_back({elem_id, side, bnd});
    }
  }

  Parallel::push_parallel_vector_data(
      bnd_info.comm(),
      add_ghost_sides,
      [&mesh, &bnd_info, this](
          processor_id_type,
          const std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>> & received)
      {
        for (const auto & [elem_id, side, bnd] : received)
          bnd_info.add_side(mesh.elemPtr(elem_id), side, bnd);
      });

  Parallel::push_parallel_vector_data(
      bnd_info.comm(),
      remove_ghost_sides,
      [&mesh, &bnd_info, this](
          processor_id_type,
          const std::vector<std::tuple<dof_id_type, unsigned short, BoundaryID>> & received)
      {
        for (const auto & [elem_id, side, bnd] : received)
          bnd_info.remove_side(mesh.elemPtr(elem_id), side, bnd);
      });

  bnd_info.parallel_sync_side_ids();
  bnd_info.parallel_sync_node_ids();
  mesh.update();
}

ConstElemRange &
ElementSubdomainModifier::activatedElemRange(bool displaced)
{
  if (!displaced && _activated_elem_range)
    return *_activated_elem_range.get();

  if (displaced && _activated_displaced_elem_range)
    return *_activated_displaced_elem_range.get();

  // Create a vector of the newly activated elements
  std::vector<Elem *> elems;
  for (auto elem_id : _activated_elems)
    elems.push_back(displaced ? _displaced_mesh->elemPtr(elem_id) : _mesh.elemPtr(elem_id));

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  if (!displaced)
    _activated_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);
  else
    _activated_displaced_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);

  return activatedElemRange(displaced);
}

ConstBndNodeRange &
ElementSubdomainModifier::activatedBndNodeRange(bool displaced)
{
  if (!displaced && _activated_bnd_node_range)
    return *_activated_bnd_node_range.get();

  if (displaced && _activated_displaced_bnd_node_range)
    return *_activated_displaced_bnd_node_range.get();

  // Create a vector of the newly activated boundary nodes
  std::vector<const BndNode *> nodes;
  auto bnd_nodes =
      displaced ? _displaced_mesh->getBoundaryNodeRange() : _mesh.getBoundaryNodeRange();
  for (auto bnd_node : *bnd_nodes)
    if (_activated_nodes.count(bnd_node->_node->id()))
      nodes.push_back(bnd_node);

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  if (!displaced)
    _activated_bnd_node_range = std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);
  else
    _activated_displaced_bnd_node_range =
        std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);

  return activatedBndNodeRange(displaced);
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
    auto displaced_elem = _displaced_problem ? _displaced_mesh->elemPtr(id) : nullptr;
    if (displaced_elem)
      displaced_elem->subdomain_id() = subdomain_id;
  }
}
