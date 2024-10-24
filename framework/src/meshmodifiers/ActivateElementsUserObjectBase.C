//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActivateElementsUserObjectBase.h"
#include "DisplacedProblem.h"

#include "libmesh/quadrature.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"
#include "libmesh/point.h"
#include "libmesh/dof_map.h"

#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/mesh_communication.h"

InputParameters
ActivateElementsUserObjectBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Determine activated elements.");
  params.addRequiredParam<subdomain_id_type>("active_subdomain_id", "The active subdomain ID.");
  params.addParam<subdomain_id_type>(
      "inactive_subdomain_id",
      libMesh::invalid_uint,
      "The inactivate subdomain ID, i.e., the subdomain that you want to keep the same.");
  params.addRequiredParam<std::vector<BoundaryName>>("expand_boundary_name",
                                                     "The expanded boundary name.");
  params.registerBase("MeshModifier");
  return params;
}

ActivateElementsUserObjectBase::ActivateElementsUserObjectBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _active_subdomain_id(declareRestartableData<subdomain_id_type>(
        "active_subdomain_id", getParam<subdomain_id_type>("active_subdomain_id"))),
    _inactive_subdomain_id(declareRestartableData<subdomain_id_type>(
        "inactive_subdomain_id", getParam<subdomain_id_type>("inactive_subdomain_id"))),
    _expand_boundary_name(getParam<std::vector<BoundaryName>>("expand_boundary_name"))
{
  setNewBoundayName();
}

void
ActivateElementsUserObjectBase::setNewBoundayName()
{
  mooseAssert(!_expand_boundary_name.empty(), "Expanded boundary name is empty");
  // add the new boundary and get its boundary id
  _boundary_ids = _mesh.getBoundaryIDs(_expand_boundary_name, true);
  mooseAssert(!_boundary_ids.empty(), "Boundary ID is empty.");
  _mesh.setBoundaryName(_boundary_ids[0], _expand_boundary_name[0]);
  _mesh.getMesh().get_boundary_info().sideset_name(_boundary_ids[0]) = _expand_boundary_name[0];
  _mesh.getMesh().get_boundary_info().nodeset_name(_boundary_ids[0]) = _expand_boundary_name[0];

  auto displaced_problem = _fe_problem.getDisplacedProblem();
  if (displaced_problem)
  {
    _disp_boundary_ids = displaced_problem->mesh().getBoundaryIDs(_expand_boundary_name, true);
    mooseAssert(!_disp_boundary_ids.empty(), "Boundary ID in the displaced mesh is empty");

    displaced_problem->mesh().setBoundaryName(_disp_boundary_ids[0], _expand_boundary_name[0]);
    displaced_problem->mesh().getMesh().get_boundary_info().sideset_name(_disp_boundary_ids[0]) =
        _expand_boundary_name[0];
    displaced_problem->mesh().getMesh().get_boundary_info().nodeset_name(_disp_boundary_ids[0]) =
        _expand_boundary_name[0];
  }
}

void
ActivateElementsUserObjectBase::execute()
{
  if (isElementActivated() && _current_elem->subdomain_id() != _active_subdomain_id &&
      _current_elem->subdomain_id() != _inactive_subdomain_id)
  {
    /*
      _current_elem subdomain id is not assignable
      create a copy of this element from MooseMesh
    */
    dof_id_type ele_id = _current_elem->id();
    Elem * ele = _mesh.elemPtr(ele_id);

    // Add element to the activate subdomain
    ele->subdomain_id() = _active_subdomain_id;

    //  Reassign element in the reference mesh while using a displaced mesh
    auto displaced_problem = _fe_problem.getDisplacedProblem();
    if (displaced_problem)
    {
      Elem * disp_ele = displaced_problem->mesh().elemPtr(ele_id);
      disp_ele->subdomain_id() = _active_subdomain_id;
    }

    // Save the newly activated element id and node for updating boundary info later
    _newly_activated_elem.insert(ele_id);
    for (unsigned int i = 0; i < ele->n_nodes(); ++i)
      _newly_activated_node.insert(ele->node_id(i));
  }
}

void
ActivateElementsUserObjectBase::finalize()
{
  /*
    Synchronize ghost element subdomain ID
    Note: this needs to be done before updating boundary info because
    updating boundary requires the updated element subdomain ids
  */
  libMesh::SyncSubdomainIds sync(_mesh.getMesh());
  Parallel::sync_dofobject_data_by_id(_mesh.getMesh().comm(),
                                      _mesh.getMesh().elements_begin(),
                                      _mesh.getMesh().elements_end(),
                                      sync);
  // Update boundary info
  updateBoundaryInfo(_mesh);

  // Similarly for the displaced mesh
  auto displaced_problem = _fe_problem.getDisplacedProblem();
  if (displaced_problem)
  {
    libMesh::SyncSubdomainIds sync_mesh(displaced_problem->mesh().getMesh());
    Parallel::sync_dofobject_data_by_id(displaced_problem->mesh().getMesh().comm(),
                                        displaced_problem->mesh().getMesh().elements_begin(),
                                        displaced_problem->mesh().getMesh().elements_end(),
                                        sync_mesh);
    updateBoundaryInfo(displaced_problem->mesh());
  }

  // Reinit equation systems
  _fe_problem.meshChanged();

  // Get storage ranges for the newly activated elements and boundary nodes
  ConstElemRange & elem_range = *this->getNewlyActivatedElementRange();
  ConstBndNodeRange & bnd_node_range = *this->getNewlyActivatedBndNodeRange();

  // Apply initial condition for the newly activated elements
  initSolutions(elem_range, bnd_node_range);

  //  Initialize stateful material properties for the newly activated elements
  _fe_problem.initElementStatefulProps(elem_range, false);

  //  Clear the list
  _newly_activated_elem.clear();
  _newly_activated_node.clear();

  _node_to_remove_from_bnd.clear();
}

void
ActivateElementsUserObjectBase::getNodesToRemoveFromBnd(std::set<dof_id_type> & remove_set,
                                                        std::set<dof_id_type> & add_set)
{
  // get the difference between the remove_set and the add_set,
  // save the difference in _node_to_remove_from_bnd
  int sz = remove_set.size() + add_set.size();
  std::vector<dof_id_type> v(sz);
  std::vector<dof_id_type>::iterator it = std::set_difference(
      remove_set.begin(), remove_set.end(), add_set.begin(), add_set.end(), v.begin());
  v.resize(it - v.begin());
  _node_to_remove_from_bnd.clear();
  for (auto id : v)
    _node_to_remove_from_bnd.insert(id);
}

void
ActivateElementsUserObjectBase::insertNodeIdsOnSide(const Elem * ele,
                                                    const unsigned short int side,
                                                    std::set<dof_id_type> & node_ids)
{
  for (unsigned int i = 0; i < ele->side_ptr(side)->n_nodes(); ++i)
    node_ids.insert(ele->side_ptr(side)->node_id(i));
}

void
ActivateElementsUserObjectBase::updateBoundaryInfo(MooseMesh & mesh)
{
  // save the removed ghost sides and associated nodes to sync across processors
  std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>>
      ghost_sides_to_remove;
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> ghost_nodes_to_remove;

  // save nodes are added and removed
  std::set<dof_id_type> add_nodes, remove_nodes;

  for (auto ele_id : _newly_activated_elem)
  {
    Elem * ele = mesh.elemPtr(ele_id);
    for (auto s : ele->side_index_range())
    {
      Elem * neighbor_ele = ele->neighbor_ptr(s);
      if (neighbor_ele == nullptr)
      {
        // add this side to boundary
        mesh.getMesh().get_boundary_info().add_side(ele, s, _boundary_ids[0]);
        insertNodeIdsOnSide(ele, s, add_nodes);
      }
      else
      {
        if (neighbor_ele->subdomain_id() != _active_subdomain_id &&
            neighbor_ele->subdomain_id() != _inactive_subdomain_id)
        {
          // add this side to boundary
          mesh.getMesh().get_boundary_info().add_side(ele, s, _boundary_ids[0]);
          insertNodeIdsOnSide(ele, s, add_nodes);
        }
        else
        {
          // remove this side from the boundary
          mesh.getMesh().get_boundary_info().remove_side(ele, s);
          insertNodeIdsOnSide(ele, s, remove_nodes);

          // remove the neighbor side from the boundary
          unsigned int neighbor_s = neighbor_ele->which_neighbor_am_i(ele);
          mesh.getMesh().get_boundary_info().remove_side(neighbor_ele, neighbor_s);
          insertNodeIdsOnSide(neighbor_ele, neighbor_s, remove_nodes);

          if (neighbor_ele->processor_id() != this->processor_id())
            ghost_sides_to_remove[neighbor_ele->processor_id()].emplace_back(neighbor_ele->id(),
                                                                             neighbor_s);
        }
      }
    }
  }
  // make sure to remove nodes that are not in the add list
  getNodesToRemoveFromBnd(remove_nodes, add_nodes);
  for (auto node_id : _node_to_remove_from_bnd)
    mesh.getMesh().get_boundary_info().remove_node(mesh.nodePtr(node_id), _boundary_ids[0]);

  // synchronize boundary information across processors
  push_boundary_side_info(mesh, ghost_sides_to_remove);
  push_boundary_node_info(mesh, ghost_nodes_to_remove);
  mesh.getMesh().get_boundary_info().parallel_sync_side_ids();
  mesh.getMesh().get_boundary_info().parallel_sync_node_ids();
  mesh.update();
}

void
ActivateElementsUserObjectBase::push_boundary_side_info(
    MooseMesh & mesh,
    std::unordered_map<processor_id_type, std::vector<std::pair<dof_id_type, unsigned int>>> &
        elems_to_push)
{
  auto elem_action_functor =
      [&mesh, this](processor_id_type,
                    const std::vector<std::pair<dof_id_type, unsigned int>> & received_elem)
  {
    // remove the side
    for (const auto & pr : received_elem)
      mesh.getMesh().get_boundary_info().remove_side(
          mesh.getMesh().elem_ptr(pr.first), pr.second, this->getExpandedBoundaryID());
  };

  Parallel::push_parallel_vector_data(
      mesh.getMesh().get_boundary_info().comm(), elems_to_push, elem_action_functor);
}

void
ActivateElementsUserObjectBase::push_boundary_node_info(
    MooseMesh & mesh,
    std::unordered_map<processor_id_type, std::vector<dof_id_type>> & nodes_to_push)
{
  auto node_action_functor =
      [&mesh, this](processor_id_type, const std::vector<dof_id_type> & received_nodes)
  {
    for (const auto & pr : received_nodes)
    {
      // remove the node
      mesh.getMesh().get_boundary_info().remove_node(mesh.getMesh().node_ptr(pr),
                                                     this->getExpandedBoundaryID());
    }
  };

  Parallel::push_parallel_vector_data(
      mesh.getMesh().get_boundary_info().comm(), nodes_to_push, node_action_functor);
}

ConstElemRange *
ActivateElementsUserObjectBase::getNewlyActivatedElementRange()
{
  // deletes the object first
  _activated_elem_range.reset();

  // create a vector of the newly activated elements
  std::vector<Elem *> elems;
  for (auto elem_id : _newly_activated_elem)
    elems.push_back(_mesh.elemPtr(elem_id));

  // Make some fake element iterators defining this vector of
  // elements
  Elem * const * elempp = const_cast<Elem * const *>(elems.data());
  Elem * const * elemend = elempp + elems.size();

  const auto elems_begin =
      MeshBase::const_element_iterator(elempp, elemend, Predicates::NotNull<Elem * const *>());

  const auto elems_end =
      MeshBase::const_element_iterator(elemend, elemend, Predicates::NotNull<Elem * const *>());
  if (!_activated_elem_range)
    _activated_elem_range = std::make_unique<ConstElemRange>(elems_begin, elems_end);

  return _activated_elem_range.get();
}

ConstBndNodeRange *
ActivateElementsUserObjectBase::getNewlyActivatedBndNodeRange()
{
  // deletes the object first
  _activated_bnd_node_range.reset();

  // create a vector of the newly activated nodes
  std::vector<const BndNode *> nodes;
  std::set<const BndNode *> set_nodes;
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (auto & bnode : bnd_nodes)
  {
    dof_id_type bnode_id = bnode->_node->id();
    auto it = _newly_activated_node.find(bnode_id);
    if (it != _newly_activated_node.end())
      set_nodes.insert(bnode);
  }

  nodes.assign(set_nodes.begin(), set_nodes.end());

  // Make some fake element iterators defining this vector of
  // nodes
  BndNode * const * nodepp = const_cast<BndNode * const *>(nodes.data());
  BndNode * const * nodeend = nodepp + nodes.size();

  const auto nodes_begin =
      MooseMesh::const_bnd_node_iterator(nodepp, nodeend, Predicates::NotNull<BndNode * const *>());

  const auto nodes_end = MooseMesh::const_bnd_node_iterator(
      nodeend, nodeend, Predicates::NotNull<BndNode * const *>());

  if (!_activated_bnd_node_range)
    _activated_bnd_node_range = std::make_unique<ConstBndNodeRange>(nodes_begin, nodes_end);

  return _activated_bnd_node_range.get();
}

ConstNodeRange *
ActivateElementsUserObjectBase::getNewlyActivatedNodeRange()
{
  // deletes the object first
  _activated_node_range.reset();

  // create a vector of the newly activated nodes
  std::vector<const Node *> nodes;
  for (auto elem_id : _newly_activated_elem)
  {
    const Node * const * elem_nodes = _mesh.elemPtr(elem_id)->get_nodes();
    unsigned int n_nodes = _mesh.elemPtr(elem_id)->n_nodes();
    for (unsigned int n = 0; n < n_nodes; ++n)
    {
      // check if all the elements connected to this node are newly activated
      const Node * nd = elem_nodes[n];
      if (isNewlyActivated(nd))
        nodes.push_back(nd);
    }
  }

  // Make some fake node iterators defining this vector of
  // nodes
  Node * const * nodepp = const_cast<Node * const *>(nodes.data());
  Node * const * nodeend = nodepp + nodes.size();

  const auto nodes_begin =
      MeshBase::const_node_iterator(nodepp, nodeend, Predicates::NotNull<Node * const *>());

  const auto nodes_end =
      MeshBase::const_node_iterator(nodeend, nodeend, Predicates::NotNull<Node * const *>());

  if (!_activated_node_range)
    _activated_node_range = std::make_unique<ConstNodeRange>(nodes_begin, nodes_end);

  return _activated_node_range.get();
}

bool
ActivateElementsUserObjectBase::isNewlyActivated(const Node * nd)
{
  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(nd->id());
  if (node_to_elem_pair != node_to_elem_map.end())
  {
    const std::vector<dof_id_type> & connected_ele_ids = node_to_elem_pair->second;
    for (auto connected_ele_id : connected_ele_ids)
    {
      // check the connected elements
      if (_mesh.elemPtr(connected_ele_id)->subdomain_id() == _inactive_subdomain_id)
        return false;
      if (_mesh.elemPtr(connected_ele_id)->subdomain_id() == _active_subdomain_id &&
          std::find(_newly_activated_elem.begin(), _newly_activated_elem.end(), connected_ele_id) ==
              _newly_activated_elem.end())
        return false;
    }
  }
  return true;
}

void
ActivateElementsUserObjectBase::initSolutions(ConstElemRange & elem_range,
                                              ConstBndNodeRange & bnd_node_range)
{
  // project initial condition to the current solution
  _fe_problem.projectInitialConditionOnCustomRange(elem_range, bnd_node_range);

  auto & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  NumericVector<Number> & current_solution = *nl.system().current_local_solution;
  NumericVector<Number> & old_solution = nl.solutionOld();
  NumericVector<Number> & older_solution = nl.solutionOlder();

  NumericVector<Number> & current_aux_solution =
      *_fe_problem.getAuxiliarySystem().system().current_local_solution;
  NumericVector<Number> & old_aux_solution = _fe_problem.getAuxiliarySystem().solutionOld();
  NumericVector<Number> & older_aux_solution = _fe_problem.getAuxiliarySystem().solutionOlder();

  DofMap & dof_map = nl.dofMap();
  DofMap & dof_map_aux = _fe_problem.getAuxiliarySystem().dofMap();

  std::set<dof_id_type> dofs, dofs_aux;
  // get dofs for the newly added elements
  for (auto & elem : elem_range)
  {
    std::vector<dof_id_type> di, di_aux;
    dof_map.dof_indices(elem, di);
    dof_map_aux.dof_indices(elem, di_aux);
    for (unsigned int i = 0; i < di.size(); ++i)
      dofs.insert(di[i]);
    for (unsigned int i = 0; i < di_aux.size(); ++i)
      dofs_aux.insert(di_aux[i]);

    di.clear();
    di_aux.clear();
  }

  // update solutions
  for (auto dof : dofs)
  {
    old_solution.set(dof, current_solution(dof));
    older_solution.set(dof, current_solution(dof));
  }
  // update aux solutions
  for (auto dof_aux : dofs_aux)
  {
    old_aux_solution.set(dof_aux, current_aux_solution(dof_aux));
    older_aux_solution.set(dof_aux, current_aux_solution(dof_aux));
  }

  dofs.clear();
  dofs_aux.clear();

  current_solution.close();
  old_solution.close();
  older_solution.close();

  current_aux_solution.close();
  old_aux_solution.close();
  older_aux_solution.close();

  _fe_problem.restoreSolutions();
}
