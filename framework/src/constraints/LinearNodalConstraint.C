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

#include "LinearNodalConstraint.h"
#include "limits.h"

/**
* The slave node variable is programmed as a linear combination of the master node variables
* (i.e, slave_var = a_1*master_var_1+ a_2*master_var_2+... + a_n*master_var_n).
* The master nodes ids and corresponding weights are required as input.
* The same linear combination applies to all slave nodes.
**/

template<>
InputParameters validParams<LinearNodalConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addRequiredParam<std::vector<unsigned int> >("master", "The master node IDs.");
  params.addParam<std::vector<unsigned int> >("slave_node_ids","The list of slave node ids");
  params.addParam<std::string>("slave_node_set", "NaN","The boundary ID associated with the slave side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  params.addRequiredParam<std::vector<Real> >("weights", "The weights associated with the master node ids. Must be of the same size as master node ids.");
  params.addParam<std::string>("formulation", "penalty", "The constraint formulation - penalty or kinematic");
  return params;
}

LinearNodalConstraint::LinearNodalConstraint(const InputParameters & parameters) :
    NodalConstraint(parameters),
    _master_node_ids(getParam<std::vector<unsigned int> >("master")),
    _slave_node_ids(getParam<std::vector<unsigned int> >("slave_node_ids")),
    _slave_node_set_id(getParam<std::string>("slave_node_set")),
    _penalty(getParam<Real>("penalty")),
    _weights(getParam<std::vector<Real> >("weights")),
    _formulation(getParam<std::string>("formulation"))
{
  if (_master_node_ids.size() != _weights.size())
    mooseError("master and weights should be of equal size.");

  if ((_formulation != "penalty") && (_formulation != "kinematic"))
    mooseError("formulation should be either penalty or kinematic.");

  std::vector<unsigned int>::iterator it;
  std::vector<unsigned int>::iterator its;

  if ((_slave_node_ids.size()==0) && (_slave_node_set_id == "NaN"))
    mooseError("Please specify slave_node_ids or slave_node_set.");
  else if ((_slave_node_ids.size()==0) && (_slave_node_set_id != "NaN"))
  {
    short _slave_boundary_id = _mesh.getBoundaryID(_slave_node_set_id);

    std::vector<dof_id_type> nodelist;
    std::vector<boundary_id_type> boundary_id_list;
    _mesh.getMesh().boundary_info->build_node_list(nodelist,boundary_id_list);

    std::vector<dof_id_type>::iterator in;
    std::vector<boundary_id_type>::iterator ib;

    std::vector<unsigned int>::iterator it;

    for (in = nodelist.begin(), ib = boundary_id_list.begin();
         in != nodelist.end() && ib != boundary_id_list.end();
         ++in, ++ib)
    {
      bool slave_local(_mesh.node(*in).processor_id() == _subproblem.processor_id());
      if (*ib == _slave_boundary_id && slave_local)
      {
        _connected_nodes.push_back(*in); //definiing slave nodes in the base class
        // Add elements connected to master node to Ghosted Elements
        for (it = _master_node_ids.begin(); it != _master_node_ids.end(); ++it)
        {
          std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[*it];
          for (unsigned int i =0; i<elems.size(); i++)
            _subproblem.addGhostedElem(elems[i]);
        }
      }
    }
  }
  else if ((_slave_node_ids.size()>0) && (_slave_node_set_id == "NaN"))
  {
    for (its = _slave_node_ids.begin(); its != _slave_node_ids.end(); ++its)
    {
      bool slave_local(_mesh.node(*its).processor_id() == _subproblem.processor_id());
      if (slave_local)
      {
        _connected_nodes.push_back(*its);
        // Add elements connected to master node to Ghosted Elements
        for (it = _master_node_ids.begin(); it != _master_node_ids.end(); ++it)
        {
          std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[*it];
          for (unsigned int i = 0; i<elems.size(); i++)
            _subproblem.addGhostedElem(elems[i]);
        }
      }
    }
  }

  // Add elements connected to slave node to Ghosted Elemets
  for (it = _master_node_ids.begin(); it != _master_node_ids.end(); ++it)
  {
    _master_node_vector.push_back(*it); //defining master nodes in base class

    bool master_local(_mesh.node(*it).processor_id() == _subproblem.processor_id());
    if (master_local)
    {
      for (its = _connected_nodes.begin(); its != _connected_nodes.end(); ++its)
      {
        std::vector<dof_id_type> & elems = _mesh.nodeToElemMap()[*its];
        for (unsigned int i = 0; i<elems.size(); i++)
          _subproblem.addGhostedElem(elems[i]);
      }
    }
  }
}

LinearNodalConstraint::~LinearNodalConstraint()
{
}

Real
LinearNodalConstraint::computeQpResidual(Moose::ConstraintType type, NumericVector<Number> & residual)
{
  /**
  * Slave residual is u_slave - weights[1]*u_master[1]-weights[2]*u_master[2] ... -u_master[n]*weights[n]
  * However, Nodal constraint residual is calculated for only a combination of one master and one slave node at a time.
  * To get around this, the residual is split up such that the final slave residual resembles the above expression.
  **/

  Real res = residual(_var.nodalDofIndexNeighbor()); //current slave node residual

  unsigned int _master_size = _master_node_ids.size(); //number of master nodes

  std::vector<unsigned int>::iterator it;
  unsigned int i = 0;
  for (it = _master_node_ids.begin(); it != _master_node_ids.end(); ++it)
  {
    if (_master_node == &(_mesh.node(*it))) //finds the position i of current master node in the vector _master_node_ids
    {
      switch (type)
      {
        case Moose::Master:
          if (_formulation == "penalty")
            return (_u_master[_qp]*_weights[i] - _u_slave[_qp]/_master_size) * _penalty;
          else if (_formulation == "kinematic")
            return res*_weights[i];

        case Moose::Slave:
          if (_formulation == "penalty")
            return (_u_slave[_qp]/_master_size - _u_master[_qp]*_weights[i]) * _penalty;
          else if (_formulation == "kinematic")
            return -res/_master_size + (_u_slave[_qp]/_master_size - _u_master[_qp]*_weights[i])*_penalty;
      }
    }
    i++;
  }
  return 0.;
}

Real
LinearNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type, SparseMatrix<Number> & jacobian)
{
  unsigned int _master_size = _master_node_ids.size();

  std::vector<unsigned int>::iterator it;
  unsigned int i = 0;
  for (it = _master_node_ids.begin(); it != _master_node_ids.end(); ++it)
  {
    if (_master_node == &(_mesh.node(*it)))
    {
      switch (type)
      {
        case Moose::MasterMaster:
          if (_formulation == "penalty")
            return _penalty*_weights[i];
          else if (_formulation == "kinematic")
            return 0.;

        case Moose::MasterSlave:
          if (_formulation == "penalty")
            return -_penalty/_master_size;
          else if (_formulation == "kinematic")
          {
            double slave_jac = (jacobian)(_var.nodalDofIndexNeighbor(), _var.nodalDofIndex()); //current jacobian entry
            return slave_jac*_weights[i];
          }

        case Moose::SlaveSlave:
          if (_formulation == "penalty")
            return _penalty/_master_size;
          else if (_formulation == "kinematic")
          {
            double curr_jac = (jacobian)(_var.nodalDofIndexNeighbor(), _var.nodalDofIndexNeighbor());
            return -curr_jac/_master_size + _penalty/_master_size;
          }

        case Moose::SlaveMaster:
          if (_formulation == "penalty")
            return -_penalty*_weights[i];
          else if (_formulation == "kinematic")
          {
            double curr_jac = (jacobian)(_var.nodalDofIndexNeighbor(), _var.nodalDofIndex());
            return -curr_jac/_master_size - _penalty*_weights[i];
          }
      }
    }
    i++;
  }
  return 0.;
}

