/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "EqualValueBoundaryConstraint.h"
#include "MooseMesh.h"

// C++ includes
#include <limits.h>

template<>
InputParameters validParams<EqualValueBoundaryConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addParam<unsigned int>("master", std::numeric_limits<unsigned int>::max(), "The ID of the master node. If no ID is provided, first node of slave set is chosen.");
  params.addParam<std::vector<unsigned int> >("slave_node_ids", "The IDs of the slave node");
  params.addParam<BoundaryName>("slave", "NaN", "The boundary ID associated with the slave side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const InputParameters & parameters) :
    NodalConstraint(parameters),
    _master_node_id(getParam<unsigned int>("master")),
    _slave_node_ids(getParam<std::vector<unsigned int> >("slave_node_ids")),
    _slave_node_set_id(getParam<BoundaryName>("slave")),
    _penalty(getParam<Real>("penalty"))
{
  if ((_slave_node_ids.size() == 0) && (_slave_node_set_id == "NaN"))
    mooseError("Please specify slave node ids or boundary id.");
  else if ((_slave_node_ids.size() == 0) && (_slave_node_set_id != "NaN"))
  {
    std::vector<dof_id_type> nodelist = _mesh.getNodeList(_mesh.getBoundaryID(_slave_node_set_id));
    std::vector<dof_id_type>::iterator in;

    //Setting master node to first node of the slave node set if no master node id is pprovided
    if (_master_node_id == std::numeric_limits<unsigned int>::max())
    {
      in = nodelist.begin();
      _master_node_id = *in;
    }
    _master_node_vector.push_back(_master_node_id); //_master_node_vector defines master nodes in the base class

    for (in = nodelist.begin(); in != nodelist.end(); ++in)
    {
      if ((*in != _master_node_id) && (_mesh.nodeRef(*in).processor_id() == _subproblem.processor_id()))
        _connected_nodes.push_back(*in); //_connected_nodes defines slave nodes in the base class
    }
  }
  else if ((_slave_node_ids.size() != 0) && (_slave_node_set_id == "NaN"))
  {
    if (_master_node_id == std::numeric_limits<unsigned int>::max())
      _master_node_id = _slave_node_ids[0];

    _master_node_vector.push_back(_master_node_id); //_master_node_vector defines master nodes in the base class

    std::vector<unsigned int>::iterator its;
    for (its = _slave_node_ids.begin(); its != _slave_node_ids.end(); ++its)
    {
      if ((_mesh.nodeRef(*its).processor_id() == _subproblem.processor_id()) && (*its != _master_node_id))
        _connected_nodes.push_back(*its);
    }
  }

  // Add elements connected to master node to Ghosted Elements
  std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[_master_node_id];
  for (unsigned int i = 0; i < elems.size(); i++)
    _subproblem.addGhostedElem(elems[i]);
}

EqualValueBoundaryConstraint::~EqualValueBoundaryConstraint()
{
}

Real
EqualValueBoundaryConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Slave:
      return (_u_slave[_i] - _u_master[_j]) * _penalty;
    case Moose::Master:
      return (_u_master[_j] - _u_slave[_i]) * _penalty;
  }
  return 0.;
}

Real
EqualValueBoundaryConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SlaveSlave:
      return _penalty;
    case Moose::SlaveMaster:
      return -_penalty;
    case Moose::MasterMaster:
      return _penalty;
    case Moose::MasterSlave:
      return -_penalty;
  }
  return 0.;
}
