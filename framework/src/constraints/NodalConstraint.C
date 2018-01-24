//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<NodalConstraint>()
{
  InputParameters params = validParams<Constraint>();
  MooseEnum formulationtype("penalty kinematic", "penalty");
  params.addParam<MooseEnum>("formulation",
                             formulationtype,
                             "Formulation used to calculate constraint - penalty or kinematic.");
  return params;
}

NodalConstraint::NodalConstraint(const InputParameters & parameters)
  : Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, true, true),
    _u_slave(_var.nodalSlnNeighbor()),
    _u_master(_var.nodalSln())
{
  MooseEnum temp_formulation = getParam<MooseEnum>("formulation");
  if (temp_formulation == "penalty")
    _formulation = Moose::Penalty;
  else if (temp_formulation == "kinematic")
    _formulation = Moose::Kinematic;
  else
    mooseError("Formulation must be either Penalty or Kinematic");
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  if ((_weights.size() == 0) && (_master_node_vector.size() == 1))
    _weights.push_back(1.0);

  std::vector<dof_id_type> masterdof = _var.dofIndices();
  std::vector<dof_id_type> slavedof = _var.dofIndicesNeighbor();
  DenseVector<Number> re(masterdof.size());
  DenseVector<Number> neighbor_re(slavedof.size());

  re.zero();
  neighbor_re.zero();

  for (_i = 0; _i < slavedof.size(); ++_i)
  {
    for (_j = 0; _j < masterdof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Penalty:
          re(_j) += computeQpResidual(Moose::Master) * _var.scalingFactor();
          neighbor_re(_i) += computeQpResidual(Moose::Slave) * _var.scalingFactor();
          break;
        case Moose::Kinematic:
          // Transfer the current residual of the slave node to the master nodes
          Real res = residual(slavedof[_i]);
          re(_j) += res * _weights[_j];
          neighbor_re(_i) += -res / _master_node_vector.size() + computeQpResidual(Moose::Slave);
          break;
      }
    }
  }
  _assembly.cacheResidualNodes(re, masterdof);
  _assembly.cacheResidualNodes(neighbor_re, slavedof);
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  if ((_weights.size() == 0) && (_master_node_vector.size() == 1))
    _weights.push_back(1.0);

  // Calculate Jacobian enteries and cache those entries along with the row and column indices
  std::vector<dof_id_type> slavedof = _var.dofIndicesNeighbor();
  std::vector<dof_id_type> masterdof = _var.dofIndices();

  DenseMatrix<Number> Kee(masterdof.size(), masterdof.size());
  DenseMatrix<Number> Ken(masterdof.size(), slavedof.size());
  DenseMatrix<Number> Kne(slavedof.size(), masterdof.size());
  DenseMatrix<Number> Knn(slavedof.size(), slavedof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();
  Knn.zero();

  for (_i = 0; _i < slavedof.size(); ++_i)
  {
    for (_j = 0; _j < masterdof.size(); ++_j)
    {
      switch (_formulation)
      {
        case Moose::Penalty:
          Kee(_j, _j) += computeQpJacobian(Moose::MasterMaster);
          Ken(_j, _i) += computeQpJacobian(Moose::MasterSlave);
          Kne(_i, _j) += computeQpJacobian(Moose::SlaveMaster);
          Knn(_i, _i) += computeQpJacobian(Moose::SlaveSlave);
          break;
        case Moose::Kinematic:
          Kee(_j, _j) = 0.;
          Ken(_j, _i) += jacobian(slavedof[_i], masterdof[_j]) * _weights[_j];
          Kne(_i, _j) += -jacobian(slavedof[_i], masterdof[_j]) / masterdof.size() +
                         computeQpJacobian(Moose::SlaveMaster);
          Knn(_i, _i) += -jacobian(slavedof[_i], slavedof[_i]) / masterdof.size() +
                         computeQpJacobian(Moose::SlaveSlave);
          break;
      }
    }
  }
  _assembly.cacheJacobianBlock(Kee, masterdof, masterdof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Ken, masterdof, slavedof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kne, slavedof, masterdof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Knn, slavedof, slavedof, _var.scalingFactor());
}

void
NodalConstraint::updateConnectivity()
{
}
