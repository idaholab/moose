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

InputParameters
ElementSubdomainModifier::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Modify element subdomain ID. This userobject only runs on the undisplaced mesh, and it will "
      "modify both the undisplaced and the displaced mesh.");
  params.addParam<bool>("apply_initial_conditions",
                        true,
                        "Whether to apply initial conditions on the moved nodes and elements");
  params.addParam<BoundaryName>(
      "moving_boundary_name",
      "Boundary to modify when an element is moved. A boundary with the provided name will be "
      "created if not already exists on the mesh.");
  params.addParam<BoundaryName>("complement_moving_boundary_name",
                                "Boundary that associated with the unmoved domain when neighbor "
                                "element(s) is moved. A boundary with the provided name will be "
                                "created if not already exists on the mesh.");
  params.set<bool>("use_displaced_mesh") = false;
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ElementSubdomainModifier::ElementSubdomainModifier(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _displaced_problem(_fe_problem.getDisplacedProblem().get()),
    _apply_ic(getParam<bool>("apply_initial_conditions")),
    _moving_boundary_specified(isParamValid("moving_boundary_name")),
    _complement_moving_boundary_specified(isParamValid("complement_moving_boundary_name"))
{
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
  _moved_nodes.clear();
  _cached_subdomain_assignments.clear();
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
    // Change the element's subdomain
    Elem * elem = _mesh.elemPtr(elem_id);
    Elem * displaced_elem =
        _displaced_problem ? _displaced_problem->mesh().elemPtr(elem_id) : nullptr;

    // Save the affected nodes so that we can later update/initialize the solution
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      _moved_nodes.insert(elem->node_id(i));

    _cached_subdomain_assignments.emplace_back(elem, subdomain_id);
    _moved_elems.push_back(elem);
    if (displaced_elem)
    {
      _cached_subdomain_assignments.emplace_back(displaced_elem, subdomain_id);
      _moved_displaced_elems.push_back(displaced_elem);
    }
  }
}

void
ElementSubdomainModifier::threadJoin(const UserObject & in_uo)
{
  // Join the data from uo into _this_ object:
  const ElementSubdomainModifier & uo = static_cast<const ElementSubdomainModifier &>(in_uo);

  _moved_elems.insert(_moved_elems.end(), uo._moved_elems.begin(), uo._moved_elems.end());

  _moved_displaced_elems.insert(_moved_displaced_elems.end(),
                                uo._moved_displaced_elems.begin(),
                                uo._moved_displaced_elems.end());

  _moved_nodes.insert(uo._moved_nodes.begin(), uo._moved_nodes.end());
  _cached_subdomain_assignments.insert(_cached_subdomain_assignments.end(),
                                       uo._cached_subdomain_assignments.begin(),
                                       uo._cached_subdomain_assignments.end());
}

void
ElementSubdomainModifier::finalize()
{
  // apply cached subdomain changes
  for (auto & [elem, subdomain_id] : _cached_subdomain_assignments)
    elem->subdomain_id() = subdomain_id;

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
  updateBoundaryInfo(_mesh, _moved_elems);

  // Similarly for the displaced mesh
  if (_displaced_problem)
  {
    SyncSubdomainIds sync_displaced(_displaced_problem->mesh().getMesh());
    Parallel::sync_dofobject_data_by_id(_displaced_problem->mesh().getMesh().comm(),
                                        _displaced_problem->mesh().getMesh().elements_begin(),
                                        _displaced_problem->mesh().getMesh().elements_end(),
                                        sync_displaced);
    updateBoundaryInfo(_displaced_problem->mesh(), _moved_displaced_elems);
  }

  // Reinit equation systems
  _fe_problem.meshChanged();

  // Apply initial condition for the newly moved elements and boundary nodes
  buildMovedElemsRange();
  buildMovedBndNodesRange();

  if (_apply_ic)
  {
    _fe_problem.projectInitialConditionOnCustomRange(movedElemsRange(), movedBndNodesRange());

    // Set old and older solution on the initialized dofs
    setOldAndOlderSolutionsForMovedNodes(_fe_problem.getNonlinearSystemBase());
    setOldAndOlderSolutionsForMovedNodes(_fe_problem.getAuxiliarySystem());
  }

  // Initialize stateful material properties for the newly activated elements
  _fe_problem.initElementStatefulProps(movedElemsRange());
}

void
ElementSubdomainModifier::updateBoundaryInfo(MooseMesh & mesh,
                                             const std::vector<const Elem *> & moved_elems)
{
  if (!_moving_boundary_specified && !_complement_moving_boundary_specified)
    return;

  BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();

  _ghost_sides_to_add.clear();
  _ghost_sides_to_remove.clear();
  _ghost_nodes_to_remove.clear();

  // The logic below updates the side set and the node set associated with the moving boundary.
  std::set<dof_id_type> added_nodes, removed_nodes;
  for (auto elem : moved_elems)
  {
    // First loop over all the sides of the element
    for (auto side : elem->side_index_range())
    {
      const Elem * neighbor = elem->neighbor_ptr(side);
      if (_complement_moving_boundary_specified)
        bnd_info.remove_side(elem, side, _complement_moving_boundary_id);
      if (neighbor && neighbor != libMesh::remote_elem)
      {
        // If the neighbor has a different subdomain ID, then this side should be added to
        // the moving boundary
        if (neighbor->subdomain_id() != elem->subdomain_id())
        {
          if (_moving_boundary_specified)
            bnd_info.add_side(elem, side, _moving_boundary_id);
          if (_complement_moving_boundary_specified)
          {
            unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);
            bnd_info.add_side(neighbor, neighbor_side, _complement_moving_boundary_id);
            if (neighbor->processor_id() != this->processor_id())
            {
              _ghost_sides_to_add[neighbor->processor_id()].emplace_back(neighbor->id(),
                                                                         neighbor_side);
            }
          }
        }
        // Otherwise remove this side and the neighbor side from the boundary.
        else
        {
          if (_moving_boundary_specified || _complement_moving_boundary_specified)
          {
            // this will destroy any overlapping third side sets!
            bnd_info.remove_side(elem, side);
            unsigned int neighbor_side = neighbor->which_neighbor_am_i(elem);
            bnd_info.remove_side(neighbor, neighbor_side);
            if (neighbor->processor_id() != this->processor_id())
              _ghost_sides_to_remove[neighbor->processor_id()].emplace_back(neighbor->id(),
                                                                            neighbor_side);
          }
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
              _ghost_nodes_to_remove[neighbor->processor_id()].push_back(elem->node_id(node));
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
    if (_moving_boundary_specified)
      mesh.getMesh().get_boundary_info().remove_node(mesh.nodePtr(node_id), _moving_boundary_id);
    if (_complement_moving_boundary_specified)
      mesh.getMesh().get_boundary_info().remove_node(mesh.nodePtr(node_id),
                                                     _complement_moving_boundary_id);
  }
  // synchronize boundary information across processors
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
      [&mesh](processor_id_type,
              const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem)
  {
    // remove the side
    for (const auto & pr : received_elem)
      mesh.get_boundary_info().remove_side(mesh.elem_ptr(pr.first), pr.second);
  };
  auto elem_add_functor =
      [&mesh, this](processor_id_type,
                    const std::vector<std::pair<dof_id_type, unsigned short int>> & received_elem)
  {
    // add the side
    for (const auto & pr : received_elem)
      mesh.get_boundary_info().add_side(
          mesh.elem_ptr(pr.first), pr.second, _complement_moving_boundary_id);
  };

  Parallel::push_parallel_vector_data(
      mesh.get_boundary_info().comm(), _ghost_sides_to_remove, elem_remove_functor);
  if (_complement_moving_boundary_specified)
    Parallel::push_parallel_vector_data(
        mesh.get_boundary_info().comm(), _ghost_sides_to_add, elem_add_functor);
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
      mesh.get_boundary_info().comm(), _ghost_nodes_to_remove, node_remove_functor);
}

void
ElementSubdomainModifier::buildMovedElemsRange()
{
  // Clear the object first
  _moved_elems_range.reset();

  // Make some fake element iterators defining this vector of elements
  Elem * const * elem_itr_begin = const_cast<Elem * const *>(_moved_elems.data());
  Elem * const * elem_itr_end = elem_itr_begin + _moved_elems.size();

  const auto elems_begin = MeshBase::const_element_iterator(
      elem_itr_begin, elem_itr_end, Predicates::NotNull<Elem * const *>());
  const auto elems_end = MeshBase::const_element_iterator(
      elem_itr_end, elem_itr_end, Predicates::NotNull<Elem * const *>());

  _moved_elems_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);
}

void
ElementSubdomainModifier::buildMovedBndNodesRange()
{
  // This is more involved than building the element range, because not all moved nodes are
  // necessarily associated with a boundary initial condition. We need to first build a set of
  // boundary nodes. Clear the object first:
  _moved_bnd_nodes_range.reset();

  // create a vector of the newly activated nodes
  std::set<const BndNode *> moved_bnd_nodes_set;
  for (auto & bnd_node : *_mesh.getBoundaryNodeRange())
  {
    dof_id_type bnd_node_id = bnd_node->_node->id();
    if (_moved_nodes.find(bnd_node_id) != _moved_nodes.end())
      moved_bnd_nodes_set.insert(bnd_node);
  }

  // Dump all the boundary nodes into a vector so that we can build a range out of it
  std::vector<const BndNode *> moved_bnd_nodes;
  moved_bnd_nodes.assign(moved_bnd_nodes_set.begin(), moved_bnd_nodes_set.end());

  // Make some fake node iterators defining this vector of nodes
  BndNode * const * bnd_node_itr_begin = const_cast<BndNode * const *>(moved_bnd_nodes.data());
  BndNode * const * bnd_node_itr_end = bnd_node_itr_begin + moved_bnd_nodes.size();

  const auto bnd_nodes_begin = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_begin, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());
  const auto bnd_nodes_end = MooseMesh::const_bnd_node_iterator(
      bnd_node_itr_end, bnd_node_itr_end, Predicates::NotNull<const BndNode * const *>());

  _moved_bnd_nodes_range = std::make_unique<ConstBndNodeRange>(bnd_nodes_begin, bnd_nodes_end);
}

void
ElementSubdomainModifier::setOldAndOlderSolutionsForMovedNodes(SystemBase & sys)
{
  // Don't do anything if this is a steady simulation
  if (!sys.hasSolutionState(1))
    return;

  ConstBndNodeRange & bnd_node_range = movedBndNodesRange();

  NumericVector<Number> & current_solution = *sys.system().current_local_solution;
  NumericVector<Number> & old_solution = sys.solutionOld();
  NumericVector<Number> * older_solution = sys.hasSolutionState(2) ? &sys.solutionOlder() : nullptr;

  DofMap & dof_map = sys.dofMap();

  // Get dofs for the newly added elements
  std::vector<dof_id_type> dofs;
  for (auto & bnd_node : bnd_node_range)
  {
    std::vector<dof_id_type> bnd_node_dofs;
    dof_map.dof_indices(bnd_node->_node, bnd_node_dofs);
    dofs.insert(dofs.end(), bnd_node_dofs.begin(), bnd_node_dofs.end());
  }

  // Set the old and older solution to match the IC.
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
