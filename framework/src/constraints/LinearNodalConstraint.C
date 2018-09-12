//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearNodalConstraint.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", LinearNodalConstraint);

template <>
InputParameters
validParams<LinearNodalConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addClassDescription(
      "Constrains slave node to move as a linear combination of master nodes.");
  params.addRequiredParam<std::vector<unsigned int>>("master", "The master node IDs.");
  params.addParam<std::vector<unsigned int>>("slave_node_ids", "The list of slave node ids");
  params.addParam<BoundaryName>(
      "slave_node_set", "NaN", "The boundary ID associated with the slave side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  params.addRequiredParam<std::vector<Real>>(
      "weights",
      "The weights associated with the master node ids. Must be of the same size as master nodes");
  return params;
}

LinearNodalConstraint::LinearNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _master_node_ids(getParam<std::vector<unsigned int>>("master")),
    _slave_node_ids(getParam<std::vector<unsigned int>>("slave_node_ids")),
    _slave_node_set_id(getParam<BoundaryName>("slave_node_set")),
    _penalty(getParam<Real>("penalty"))
{
  _weights = getParam<std::vector<Real>>("weights");

  if (_master_node_ids.size() != _weights.size())
    mooseError("master and weights should be of equal size.");

  if ((_slave_node_ids.size() == 0) && (_slave_node_set_id == "NaN"))
    mooseError("Please specify slave_node_ids or slave_node_set.");
  else if ((_slave_node_ids.size() == 0) && (_slave_node_set_id != "NaN"))
  {
    std::vector<dof_id_type> nodelist = _mesh.getNodeList(_mesh.getBoundaryID(_slave_node_set_id));
    std::vector<dof_id_type>::iterator in;

    for (in = nodelist.begin(); in != nodelist.end(); ++in)
    {
      if (_mesh.nodeRef(*in).processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(*in); // defining slave nodes in the base class
    }
  }
  else if ((_slave_node_ids.size() > 0) && (_slave_node_set_id == "NaN"))
  {
    for (const auto & dof : _slave_node_ids)
      if (_mesh.nodeRef(dof).processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(dof);
  }

  const auto & node_to_elem_map = _mesh.nodeToElemMap();

  // Add elements connected to master node to Ghosted Elements
  for (const auto & dof : _master_node_ids)
  {
    // defining master nodes in base class
    _master_node_vector.push_back(dof);

    auto node_to_elem_pair = node_to_elem_map.find(dof);
    mooseAssert(node_to_elem_pair != node_to_elem_map.end(), "Missing entry in node to elem map");
    const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

    for (const auto & elem_id : elems)
      _subproblem.addGhostedElem(elem_id);
  }
}

Real
LinearNodalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  /**
  * Slave residual is u_slave - weights[1]*u_master[1]-weights[2]*u_master[2] ...
  *-u_master[n]*weights[n]
  * However, computeQPresidual is calculated for only a combination of one master and one slave node
  *at a time.
  * To get around this, the residual is split up such that the final slave residual resembles the
  *above expression.
  **/

  unsigned int master_size = _master_node_ids.size();

  switch (type)
  {
    case Moose::Master:
      return (_u_master[_j] * _weights[_j] - _u_slave[_i] / master_size) * _penalty;
    case Moose::Slave:
      return (_u_slave[_i] / master_size - _u_master[_j] * _weights[_j]) * _penalty;
  }
  return 0.;
}

Real
LinearNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  unsigned int master_size = _master_node_ids.size();

  switch (type)
  {
    case Moose::MasterMaster:
      return _penalty * _weights[_j];
    case Moose::MasterSlave:
      return -_penalty / master_size;
    case Moose::SlaveSlave:
      return _penalty / master_size;
    case Moose::SlaveMaster:
      return -_penalty * _weights[_j];
  }
  return 0.;
}
