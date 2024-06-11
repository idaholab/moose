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
      "Moving boundaries. A boundary with the provided name will be created if one does not "
      "already exist. These boundaries will be updated as elements change their subdomain. The "
      "corresponding subdomains of each moving boundary shall be specified using the parameter "
      "'moving_boundary_subdomains'.");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "moving_boundary_subdomains", {}, "The subdomains associated with each moving boundary.");
  params.addParam<BoundaryName>("default_moving_boundary",
                                "default_moving_boundary",
                                "Name of the default moving boundary covering subdomains not "
                                "specified in 'moving_boundary_subdomains'.");

  params.addParam<std::vector<SubdomainName>>(
      "active_subdomains",
      {},
      "The 'active' subdomains on which the simulation is performed, i.e. the PDE computational "
      "domain. If this parameter is left empty, then the entire mesh is treated as the active "
      "subdomain.");
  params.addParam<std::vector<VariableName>>(
      "initialize_variables",
      {},
      "Variables to initialize when an element is moved into the active subdomains.");

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
    _init_vars(getParam<std::vector<VariableName>>("initialize_variables"))
{
}

void
ElementSubdomainModifier::initialSetup()
{
  for (auto var_name : _init_vars)
    if (!_nl_sys.hasVariable(var_name) && !_aux_sys.hasVariable(var_name))
      paramError("initialize_variables", "Variable ", var_name, " does not exist.");

  // Initialize moving boundaries
  const auto bnd_names = getParam<std::vector<BoundaryName>>("moving_boundaries");
  bnd_names.push_back(getParam<BoundaryName>("default_moving_boundary"));
  const auto bnd_ids = _mesh.getBoundaryIDs(bnd_names, true);
  const auto bnd_subdomains =
      getParam<std::vector<std::vector<SubdomainName>>>("moving_boundary_subdomains");
  for (auto i : index_range(bnd_names))
    _moving_boundaries.emplace_back(
        bnd_names[i], bnd_ids[i], _mesh.getSubdomainIDs(bnd_subdomains[i]));

  // Create moving boundaries onto the undisplaced and displaced meshes
  createMovingBoundaries(_mesh);
  if (_displaced_mesh)
    createMovingBoundaries(*_displaced_mesh);
}

void
ElementSubdomainModifier::createMovingBoundaries(MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();
  for (const auto & bnd : _moving_boundaries)
  {
    bnd_info.sideset_name(bnd.id) = bnd.name;
    bnd_info.nodeset_name(bnd.id) = bnd.name;
  }
}

void
ElementSubdomainModifier::initialize()
{
  _moved_elems.clear();
  _activated_elems.clear();
  _activated_nodes.clear();
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
    _moved_elems.emplace(elem_id, subdomain_id);

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
ElementSubdomainModifier::subdomainBelongsTo(SubdomainID id,
                                             const std::vector<SubdomainID> & set) const
{
  return std::find(set.begin(), set.end(), id) != set.end();
}

bool
ElementSubdomainModifier::subdomainIsActive(SubdomainID id) const
{
  return subdomainBelongsTo(id, _active_subdomains);
}

BoundaryID
ElementSubdomainModifier::subdomainMovingBoundary(SubdomainID id) const
{
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

  // 1. Apply cached subdomain changes
  // 2. Synchronize ghost element subdomain ID
  // 3. Update moving boundary
  applySubdomainChanges(_mesh);
  updateMovingBoundaryInfo(_mesh);

  // Similarly for the displaced mesh
  if (_displaced_mesh)
  {
    applySubdomainChanges(*_displaced_mesh);
    updateMovingBoundaryInfo(*_displaced_mesh);
  }

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
  for (const auto & [elem_id, subdomain_id] : _moved_elems)
    mesh.elemPtr(elem_id)->subdomain_id() = subdomain_id;

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
  // Clear cached ranged
  _activated_elem_range.reset();
  _activated_displaced_elem_range.reset();
  _activated_bnd_node_range.reset();
  _activated_displaced_bnd_node_range.reset();
}

void
ElementSubdomainModifier::updateMovingBoundaryInfo(const MovingBoundary & bnd, MooseMesh & mesh)
{
  auto & bnd_info = mesh.getMesh().get_boundary_info();

  // Names might have been deleted in the previous step if there is no side on this boundary
  bnd_info.sideset_name(_moving_boundary_id) = _moving_boundary_name;

  // Go through moved elements to figure out element sides and nodes to be added/removed from
  // the moving boundary
  std::unordered_map<dof_id_type, std::unordered_map<unsigned short, BoundaryID>> add_sides,
      remove_sides;
  for (const auto & [elem_id, subdomain_id] : _moved_elems)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (auto side : elem->side_index_range())
    {
      auto neigh = elem->neighbor_ptr(side);
      if (!neigh)
      {
        std::cout << "   to be added." << std::endl;
        add_sides[elem->id()].push_back(side);
      }
      else if (neigh->active() && neigh != libMesh::remote_elem)
      {
        if (subdomainIsActive(neigh->subdomain_id()))
        {
          std::cout << "   to be removed." << std::endl;
          remove_sides[neigh->id()].push_back(neigh->which_neighbor_am_i(elem));
        }
        else
        {
          std::cout << "   to be added." << std::endl;
          add_sides[elem->id()].push_back(side);
        }
      }
      else if (!neigh->active() && neigh != libMesh::remote_elem)
      {
        std::vector<const Elem *> neigh_children;
        neigh->top_parent()->active_family_tree_by_neighbor(neigh_children, elem);

        bool neigh_children_on_active_subdomain =
            std::all_of(neigh_children.begin(),
                        neigh_children.end(),
                        [this](const Elem * neigh_child)
                        { return subdomainIsActive(neigh_child->subdomain_id()); });

        bool neigh_children_on_inactive_subdomain =
            std::all_of(neigh_children.begin(),
                        neigh_children.end(),
                        [this](const Elem * neigh_child)
                        { return !subdomainIsActive(neigh_child->subdomain_id()); });

        if (neigh_children_on_active_subdomain)
          for (auto neigh_child : neigh_children)
            remove_sides[neigh_child->id()].push_back(neigh_child->which_neighbor_am_i(elem));
        else if (neigh_children_on_inactive_subdomain)
          add_sides[elem->id()].push_back(side);
        else
          mooseException(
              "The neighbor element of a newly activated element ",
              elem->id(),
              " has been adaptively refined. Part of the neighbor element is on the "
              "active subdomain, while other part is on the inactive subdomain. It is therefore "
              "not possible to update the element side on the moving boundary consistently.");
      }
    }
  }

  // Add element sides to moving boundary
  for (const auto & [elem_id, sides] : add_sides)
    for (const auto & side : sides)
    {
      bnd_info.add_side(mesh.elemPtr(elem_id), side, _moving_boundary_id);
      std::cout << "Added element " << elem_id << " side " << side << std::endl;
    }

  // Remove element sides from moving boundary
  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned short>>>
      remove_ghosted_sides;
  for (const auto & [elem_id, sides] : remove_sides)
  {
    auto elem = mesh.elemPtr(elem_id);
    for (const auto & side : sides)
    {
      bnd_info.remove_side(elem, side, _moving_boundary_id);
      std::cout << "Removed element " << elem_id << " side " << side << std::endl;
      if (elem->processor_id() != processor_id())
        remove_ghosted_sides[elem->processor_id()].push_back({elem_id, side});
    }
  }
  Parallel::push_parallel_vector_data(
      bnd_info.comm(),
      remove_ghosted_sides,
      [&mesh, &bnd_info, this](processor_id_type,
                               const std::vector<std::pair<dof_id_type, unsigned short>> & received)
      {
        for (const auto & [elem_id, side] : received)
          bnd_info.remove_side(mesh.elemPtr(elem_id), side, _moving_boundary_id);
      });

  mesh.getMesh().get_boundary_info().parallel_sync_side_ids();
  mesh.getMesh().get_boundary_info().parallel_sync_node_ids();
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
