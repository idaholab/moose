//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVoidVolume.h"
#include "Assembly.h"
#include "libmesh/parallel_sync.h"

registerMooseObject("GeochemistryApp", NodalVoidVolume);

InputParameters
NodalVoidVolume::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addCoupledVar("porosity", 1.0, "Porosity");
  params.addCoupledVar(
      "concentration",
      "A concentration variable.  This is only used to determine the finite-element type of your "
      "concentration variable.  The default is linear Lagrange.  Therefore, if you are using "
      "linear-lagrange variables you do not need to supply this input");
  // All elements that share a node with the elements owned by this processor are known about by
  // this UserObject
  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 1; });
  params.addClassDescription(
      "UserObject to compute the nodal void volume.  Take care if you block-restrict this "
      "UserObject, since the volumes of the nodes on the block's boundary will not include any "
      "contributions from outside the block.");
  return params;
}

NodalVoidVolume::NodalVoidVolume(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _porosity(coupledValue("porosity")),
    _rebuilding_needed(true),
    _my_pid(processor_id()),
    _phi(isParamValid("concentration") ? getVar("concentration", 0)->phi()
                                       : _assembly.fePhi<Real>(FEType(1, FEFamily::LAGRANGE)))
{
}

void
NodalVoidVolume::initialSetup()
{
  ElementUserObject::initialSetup();
  if (_rebuilding_needed)
    rebuildStructures(); // reinitialize _nodal_void_volume and rebuild MPI communication lists
}

void
NodalVoidVolume::meshChanged()
{
  ElementUserObject::meshChanged();
  _rebuilding_needed = true; // signal that internal datastructures need rebuilding
}

void
NodalVoidVolume::timestepSetup()
{
  ElementUserObject::timestepSetup();
  if (_rebuilding_needed)
    rebuildStructures(); // reinitialize _nodal_void_volume and rebuild MPI communication lists
}

void
NodalVoidVolume::rebuildStructures()
{
  // Because of the RelationshipManager, this processor knows about all its elements as well as 1
  // layer of ghost elements.  Hence, the loop over getNonlinearEvaluableElementRange below goes
  // over all the local elements as well as 1 layer of ghost elements, and all their nodes are
  // recorded.  The ghosted elements are not visited in execute() so the nodal volume is incorrectly
  // computed for all the nodes belonging to ghosted elements.  So MPI communication of nodal volume
  // info is needed (implemented in exchangeGhostedInfo).
  _nodal_void_volume.clear();
  for (const auto & elem : _fe_problem.getNonlinearEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
      for (const auto & node : elem->node_ref_range())
        _nodal_void_volume[&node] = 0.0;
  if (_app.n_processors() > 1)
    buildCommLists();
  _rebuilding_needed = false;
}

void
NodalVoidVolume::buildCommLists()
{
  // global_node_nums_to_receive[pid] is a list of node numbers connected to elements owned by
  // processor pid.  _nodes_to_receive[pid] is a list of node pointers.  Both lists have the same
  // ordering
  std::map<processor_id_type, std::vector<dof_id_type>> global_node_nums_to_receive;
  _nodes_to_receive.clear();
  // seen_nodes are those that have been seen to be attached to an element of given processor ID
  std::map<processor_id_type, std::set<const Node *>> seen_nodes;
  // run through all elements known by this processor (the owned + 1 layer of ghosted elements),
  // recording nodes that are attached to elements that aren't owned by this processor
  for (const auto & elem : _fe_problem.getNonlinearEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
    {
      const processor_id_type elem_pid = elem->processor_id();
      if (elem_pid != _my_pid)
      {
        auto & pid_nodes = seen_nodes[elem_pid];
        for (const auto & node : elem->node_ref_range())
          if (pid_nodes.insert(&node).second) // true if the node has not been seen
          {
            global_node_nums_to_receive[elem_pid].push_back(node.id());
            _nodes_to_receive[elem_pid].push_back(&node);
          }
      }
    }

  // exchange this info with other processors, building global_node_nums_to_send at the same time
  std::map<processor_id_type, std::vector<dof_id_type>> global_node_nums_to_send;
  auto nodes_action_functor = [&](processor_id_type pid, const std::vector<dof_id_type> & nts)
  { global_node_nums_to_send[pid] = nts; };
  Parallel::push_parallel_vector_data(
      this->comm(), global_node_nums_to_receive, nodes_action_functor);

  // Build _nodes_to_send using global_node_nums_to_send, keeping the same ordering in the
  // std::vector
  _nodes_to_send.clear();
  for (const auto & kv : global_node_nums_to_send)
  {
    const processor_id_type pid = kv.first;
    auto & pid_entry = _nodes_to_send[pid];
    pid_entry.reserve(kv.second.size());
    for (const auto & node_num : kv.second)
      pid_entry.push_back(_mesh.nodePtr(node_num));
  }
}

void
NodalVoidVolume::exchangeGhostedInfo()
{
  // Exchange ghosted _nodal_void_volume information with other processors
  std::map<processor_id_type, std::vector<Real>> nvv_to_send;
  for (const auto & kv : _nodes_to_send)
  {
    const processor_id_type pid = kv.first;
    auto & pid_entry = nvv_to_send[pid];
    pid_entry.reserve(kv.second.size());
    for (const auto & nd : kv.second)
      pid_entry.push_back(_nodal_void_volume.at(nd));
  }

  auto nvv_action_functor = [this](processor_id_type pid, const std::vector<Real> & nvv_received)
  {
    const std::size_t msg_size = nvv_received.size();
    auto & receive_pid_entry = _nodes_to_receive[pid];
    mooseAssert(msg_size == receive_pid_entry.size(),
                "Message size, " << msg_size
                                 << ", incompatible with nodes_to_receive, which has size "
                                 << receive_pid_entry.size());
    for (std::size_t i = 0; i < msg_size; ++i)
      _nodal_void_volume[receive_pid_entry[i]] += nvv_received[i];
  };
  Parallel::push_parallel_vector_data(this->comm(), nvv_to_send, nvv_action_functor);
}

void
NodalVoidVolume::initialize()
{
  for (auto & nvv : _nodal_void_volume)
    nvv.second = 0.0;
}

void
NodalVoidVolume::finalize()
{
  if (_app.n_processors() > 1)
    exchangeGhostedInfo();
  // Now _nodal_void_volume is correct for all nodes within and on the boundary of this processor's
  // domain.
}

void
NodalVoidVolume::threadJoin(const UserObject & uo)
{
  // _nodal_void_volume will have been computed by other threads: add their contributions to ours.
  const NodalVoidVolume & nvv = static_cast<const NodalVoidVolume &>(uo);
  for (auto & our_nvv : _nodal_void_volume)
    our_nvv.second += nvv._nodal_void_volume.at(our_nvv.first);
  // Now _nodal_void_volume is correct for all nodes within this processor's domain (but potentially
  // not those on the boundary: exchangeGhostedInfo() will fix that)
}

void
NodalVoidVolume::execute()
{
  // this gets called for all elements owned by this processor.  So, after threadJoin(), it
  // correctly computes _nodal_void_volume for nodes compltely within this processor's domain.
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const Node * node = _current_elem->node_ptr(i);
    mooseAssert(_nodal_void_volume.count(node) == 1, "Node missing in _nodal_void_volume map");
    auto & nvv = _nodal_void_volume[node];
    for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
      nvv += _JxW[qp] * _coord[qp] * _phi[i][qp] * _porosity[qp];
  }
}

Real
NodalVoidVolume::getNodalVoidVolume(const Node * node) const
{
  auto find = _nodal_void_volume.find(node);
  if (find != _nodal_void_volume.end())
    return find->second;
  mooseError("nodal id ",
             node->id(),
             " not in NodalVoidVolume's data structures.  Perhaps the execute_on parameter of "
             "NodalVoidVolume needs to be set differently");
}
