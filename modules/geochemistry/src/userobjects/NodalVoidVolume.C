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
                                [](const InputParameters &, InputParameters & rm_params) {
                                  rm_params.set<unsigned short>("layers") = 1;
                                });
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
    _num_nodes(0),
    _my_node_number(),
    _nodal_void_volume(0),
    _my_pid(processor_id()),
    _nodes_to_receive(),
    _nodes_to_send(),
    _phi(_assembly.fePhi<Real>(isParamValid("concentration") ? getVar("concentration", 0)->feType()
                                                             : FEType(1, FEFamily::LAGRANGE)))
{
}

void
NodalVoidVolume::initialSetup()
{
  ElementUserObject::initialSetup();
  if (_rebuilding_needed)
    rebuildStructures(); // rebuild _my_node_number and MPI communication lists
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
    rebuildStructures(); // rebuild _my_node_number and MPI communication lists
}

void
NodalVoidVolume::rebuildStructures()
{
  // Because of the RelationshipManager, this processor knows about all its elements as well as 1
  // layer of ghost elements.  Hence, the loop over getEvaluableElementRange below goes over all the
  // local elements as well as 1 layer of ghost elements, and all their nodes are recorded.  The
  // ghosted elements are not visited in execute() so the nodal volume is incorrectly computed for
  // all the nodes belonging to ghosted elements.  So MPI communication of nodal volume info is
  // needed (implemented in exchangeGhostedInfo).
  _num_nodes = 0;
  _my_node_number.clear();
  for (const auto & elem : _fe_problem.getEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
      for (unsigned i = 0; i < elem->n_nodes(); ++i)
        if (_my_node_number.count(elem->node_id(i)) == 0)
          _my_node_number[elem->node_id(i)] = _num_nodes++;
  _nodal_void_volume.assign(_num_nodes, 0.0);
  if (_app.n_processors() > 1)
    buildCommLists();
  _rebuilding_needed = false;
}

void
NodalVoidVolume::buildCommLists()
{
  _nodes_to_receive.clear();

  // run through all elements known by this processor (the owned + 1 layer of ghosted elements),
  // recording nodes that are attached to elements that aren't owned by this processor
  for (const auto & elem : _fe_problem.getEvaluableElementRange())
    if (this->hasBlocks(elem->subdomain_id()))
    {
      const processor_id_type elem_pid = elem->processor_id();
      if (elem_pid != _my_pid)
      {
        if (_nodes_to_receive.find(elem_pid) == _nodes_to_receive.end())
          _nodes_to_receive[elem_pid] = std::vector<dof_id_type>();
        for (unsigned i = 0; i < elem->n_nodes(); ++i)
          if (std::find(_nodes_to_receive[elem_pid].begin(),
                        _nodes_to_receive[elem_pid].end(),
                        elem->node_id(i)) == _nodes_to_receive[elem_pid].end())
            _nodes_to_receive[elem_pid].push_back(elem->node_id(i));
      }
    }
  // nodes_to_receive[pid] is a list of nodes connected to elements owned by processor pid

  // exchange this info with other processors, building _nodes_to_send at the same time
  _nodes_to_send.clear();
  auto nodes_action_functor = [this](processor_id_type pid, const std::vector<dof_id_type> & nts) {
    _nodes_to_send[pid] = nts;
  };
  Parallel::push_parallel_vector_data(this->comm(), _nodes_to_receive, nodes_action_functor);

  // At the moment, _nodes_to_send and _nodes_to_receive contain global node numbers
  // It is slightly more efficient to convert to local node numbers
  // so that we don't have to keep doing things like
  // _u_nodal[_my_node_number.at(_nodes_to_send[pid][i])]
  // every time we send/receive
  for (auto & kv : _nodes_to_send)
  {
    const processor_id_type pid = kv.first;
    const std::size_t num_nodes = kv.second.size();
    for (unsigned i = 0; i < num_nodes; ++i)
      _nodes_to_send[pid][i] = _my_node_number.at(_nodes_to_send[pid][i]);
  }
  for (auto & kv : _nodes_to_receive)
  {
    const processor_id_type pid = kv.first;
    const std::size_t num_nodes = kv.second.size();
    for (unsigned i = 0; i < num_nodes; ++i)
      _nodes_to_receive[pid][i] = _my_node_number.at(_nodes_to_receive[pid][i]);
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
    nvv_to_send[pid] = std::vector<Real>();
    for (const auto & nd : kv.second)
      nvv_to_send[pid].push_back(_nodal_void_volume[nd]);
  }

  auto nvv_action_functor = [this](processor_id_type pid, const std::vector<Real> & nvv_received) {
    const std::size_t msg_size = nvv_received.size();
    mooseAssert(msg_size == _nodes_to_receive[pid].size(),
                "Message size, " << msg_size
                                 << ", incompatible with nodes_to_receive, which has size "
                                 << _nodes_to_receive[pid].size());
    for (unsigned i = 0; i < msg_size; ++i)
      _nodal_void_volume[_nodes_to_receive[pid][i]] += nvv_received[i];
  };
  Parallel::push_parallel_vector_data(this->comm(), nvv_to_send, nvv_action_functor);
}

void
NodalVoidVolume::initialize()
{
  _nodal_void_volume.assign(_num_nodes, 0.0);
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
  for (unsigned i = 0; i < _num_nodes; ++i)
    _nodal_void_volume[i] += nvv._nodal_void_volume[i];
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
    const dof_id_type global_node_number = _current_elem->node_id(i);
    const unsigned my_node_number = _my_node_number.at(global_node_number);
    for (unsigned qp = 0; qp < _qrule->n_points(); ++qp)
      _nodal_void_volume[my_node_number] += _JxW[qp] * _coord[qp] * _phi[i][qp] * _porosity[qp];
  }
}

Real
NodalVoidVolume::getNodalVoidVolume(unsigned node_id) const
{
  try
  {
    return _nodal_void_volume[_my_node_number.at(node_id)];
  }
  catch (std::out_of_range)
  {
    mooseError("nodal id ",
               node_id,
               " not in NodalVoidVolume's data structures.  Perhaps the execute_on parameter of "
               "NodalVoidVolume needs to be set differently");
  }
  catch (...)
  {
    throw;
  }
}
