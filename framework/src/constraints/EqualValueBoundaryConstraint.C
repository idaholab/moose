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

#include "EqualValueBoundaryConstraint.h"

template<>
InputParameters validParams<EqualValueBoundaryConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const InputParameters & parameters) :
    NodalConstraint(parameters),
    _slave_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
    _penalty(getParam<Real>("penalty"))
{

  std::vector<dof_id_type> nodelist;
  std::vector<boundary_id_type> boundary_id_list;
  _mesh.getMesh().boundary_info->build_node_list(nodelist,boundary_id_list);

  std::vector<dof_id_type>::iterator in;
  std::vector<boundary_id_type>::iterator ib;

  for (in = nodelist.begin(), ib = boundary_id_list.begin();
       in != nodelist.end() && ib != boundary_id_list.end();
       ++in, ++ib)
  {
    bool slave_local(_mesh.node(*in).processor_id() == _subproblem.processor_id());
    if (*ib == _slave_boundary_id &&
        *in != _master_node_id &&
        slave_local)
    {
      _connected_nodes.push_back(*in);

      std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[_master_node_id];
      for (unsigned int i = 0; i < elems.size(); ++i)
        _subproblem.addGhostedElem(elems[i]);
    }
  }
}

EqualValueBoundaryConstraint::~EqualValueBoundaryConstraint()
{
}

Real
EqualValueBoundaryConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
  case Moose::Master:
    return (_u_master[_qp] - _u_slave[_qp]) * _penalty;

  case Moose::Slave:
    return (_u_slave[_qp] - _u_master[_qp]) * _penalty;
  }

  return 0.;
}

Real
EqualValueBoundaryConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
  case Moose::MasterMaster:
    return _penalty;

  case Moose::MasterSlave:
    return -_penalty;

  case Moose::SlaveSlave:
    return _penalty;

  case Moose::SlaveMaster:
    return -_penalty;
  }

  return 0.;
}



// DEPRECATED CONSTRUCTOR
EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const std::string & deprecated_name, InputParameters deprecated_parameters) :
    NodalConstraint(deprecated_name, deprecated_parameters),
    _slave_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
    _penalty(getParam<Real>("penalty"))
{

  std::vector<dof_id_type> nodelist;
  std::vector<boundary_id_type> boundary_id_list;
  _mesh.getMesh().boundary_info->build_node_list(nodelist,boundary_id_list);

  std::vector<dof_id_type>::iterator in;
  std::vector<boundary_id_type>::iterator ib;

  for (in = nodelist.begin(), ib = boundary_id_list.begin();
       in != nodelist.end() && ib != boundary_id_list.end();
       ++in, ++ib)
  {
    bool slave_local(_mesh.node(*in).processor_id() == _subproblem.processor_id());
    if (*ib == _slave_boundary_id &&
        *in != _master_node_id &&
        slave_local)
    {
      _connected_nodes.push_back(*in);

      std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[_master_node_id];
      for (unsigned int i = 0; i < elems.size(); ++i)
        _subproblem.addGhostedElem(elems[i]);
    }
  }
}
