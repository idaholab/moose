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
#include "limits.h"

template<>
InputParameters validParams<EqualValueBoundaryConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addParam<unsigned int>("master",std::numeric_limits<unsigned int>::max(), "The ID of the master node. If no ID is provided, first node of slave set is chosen.");
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  params.addParam<std::string>("formulation", "penalty", "The constraint formulation - penalty or kinematic");
  return params;
}

EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const InputParameters & parameters) :
    NodalConstraint(parameters),
    _master_node_id(getParam<unsigned int>("master")),
    _slave_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("slave"))),
    _penalty(getParam<Real>("penalty")),
    _formulation(getParam<std::string>("formulation"))
{

  if ((_formulation != "penalty") && (_formulation != "kinematic"))
    mooseError("formulation should be either penalty or kinematic.");

  std::vector<dof_id_type> nodelist;
  std::vector<boundary_id_type> boundary_id_list;
  _mesh.getMesh().boundary_info->build_node_list(nodelist,boundary_id_list);

  std::vector<dof_id_type>::iterator in;
  std::vector<boundary_id_type>::iterator ib;

  //Setting master node to first node of the slave node set if no master node id is pprovided
  if (_master_node_id == std::numeric_limits<unsigned int>::max())
  {
    in = nodelist.begin();
    _master_node_id = *in;
  }
  _master_node_vector.push_back(_master_node_id); //_master_node_vector defines master nodes in the base class

  for (in = nodelist.begin(), ib = boundary_id_list.begin();
       in != nodelist.end() && ib != boundary_id_list.end();
       ++in, ++ib)
  {
    bool slave_local(_mesh.node(*in).processor_id() == _subproblem.processor_id());
    if (*ib == _slave_boundary_id &&
        *in != _master_node_id &&
        slave_local)
    {
      _connected_nodes.push_back(*in); //_connected_nodes defines slave nodes in the base class

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
EqualValueBoundaryConstraint::computeQpResidual(Moose::ConstraintType type, NumericVector<Number> & residual)
{
  Real res = residual(_var.nodalDofIndexNeighbor()); // obtain current residual of slave node

  switch (type)
  {
    case Moose::Slave:
    {
      if (_formulation == "penalty")
        return (_u_slave[_qp] - _u_master[_qp]) * _penalty;
      else if (_formulation == "kinematic")
        return -res + (_u_slave[_qp] - _u_master[_qp]) * _penalty;
    }
    case Moose::Master:
    {
      if (_formulation == "penalty")
        return (_u_master[_qp] - _u_slave[_qp]) * _penalty;
      else if (_formulation == "kinematic")
        return res;
    }
  }
  return 0.;
}

Real
EqualValueBoundaryConstraint::computeQpJacobian(Moose::ConstraintJacobianType type, SparseMatrix<Number> & jacobian)
{

  switch (type)
  {

    case Moose::SlaveSlave:
    {
      if (_formulation == "penalty")
        return _penalty;
      else if (_formulation == "kinematic")
      {
        double curr_jac = (jacobian)(_var.nodalDofIndexNeighbor(), _var.nodalDofIndexNeighbor()); //obtain current jacobian entry for slave,slave
        return -curr_jac + _penalty;
      }
    }

    case Moose::SlaveMaster:
    {
      if (_formulation == "penalty")
        return -_penalty;
      else if (_formulation == "kinematic")
      {
        double curr_jac = (jacobian)(_var.nodalDofIndexNeighbor(), _var.nodalDofIndex());
        return -curr_jac - _penalty;
      }
    }

    case Moose::MasterMaster:
    {
      if (_formulation == "penalty")
        return _penalty;
      else if (_formulation == "kinematic")
        return 0.;
    }

    case Moose::MasterSlave:
    {
      if (_formulation == "penalty")
        return -_penalty;
      else if (_formulation == "kinematic")
      {
        double slave_jac = (jacobian)(_var.nodalDofIndexNeighbor(),_var.nodalDofIndex());
        return slave_jac;
      }
    }
  }
  return 0.;
}
