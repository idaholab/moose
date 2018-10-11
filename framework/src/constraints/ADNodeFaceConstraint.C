//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNodeFaceConstraint.h"

defineADBaseValidParams(ADNodeFaceConstraint, NodeFaceConstraintBase, );

template <ComputeStage compute_stage>
ADNodeFaceConstraint<compute_stage>::ADNodeFaceConstraint(const InputParameters & parameters)
  : NodeFaceConstraintBase(parameters),
    _u_slave(_var.adDofValues<compute_stage>()),
    _u_master(_master_var.adSlnNeighbor<compute_stage>()),
    _grad_u_master(_master_var.adGradSlnNeighbor<compute_stage>())
{
}

template <ComputeStage compute_stage>
void
ADNodeFaceConstraint<compute_stage>::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_master_var.number());

  _qp = 0;

  for (_i = 0; _i < _test_master.size(); _i++)
    neighbor_re(_i) += computeQpResidual(Moose::Master);

  _i = 0;
  re(0) = computeQpResidual(Moose::Slave);
}

template <>
void
ADNodeFaceConstraint<JACOBIAN>::computeResidual()
{
}

template <ComputeStage compute_stage>
void
ADNodeFaceConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _master_var.number());

  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(
      Moose::NeighborNeighbor, _master_var.number(), _master_var.number());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _qp = 0;

  size_t slave_ad_offset = _var.number() * _sys.getMaxVarNDofsPerElem();
  size_t master_ad_offset = _master_var.number() * _sys.getMaxVarNDofsPerElem();

  for (_i = 0; _i < _test_slave.size(); _i++)
  {
    ADReal residual = computeQpResidual(Moose::Slave);
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += residual.derivatives()[slave_ad_offset + _j];
    if (Ken.m() && Ken.n())
      for (_j = 0; _j < _master_var.phiSize(); _j++)
        Ken(_i, _j) += residual.derivatives()[master_ad_offset + _j];
  }

  for (_i = 0; _i < _test_master.size(); _i++)
  {
    ADReal residual = computeQpResidual(Moose::Master);
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kne(_i, _j) += residual.derivatives()[slave_ad_offset + _j];
    if (Knn.m() && Knn.n())
      for (_j = 0; _j < _master_var.phiSize(); _j++)
        Knn(_i, _j) += residual.derivatives()[master_ad_offset + _j];
  }
}

void
ADNodeFaceConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  getConnectedDofIndices(jvar);

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);
  DenseMatrix<Number> & Knn =
      _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _master_var.number(), jvar);

  _qp = 0;

  size_t ad_offset = jvar * _sys.getMaxVarNDofsPerElem();

  for (_i = 0; _i < _test_slave.size(); _i++)
  {
    ADReal residual = computeQpResidual(Moose::Slave);
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += residual.derivatives()[ad_offset + _j];
    for (_j = 0; _j < _sys.getVariable(0, jvar).size(); _j++)
      Ken(_i, _j) += residual.derivatives()[ad_offset + _j];
  }

  if (_Kne.m() && _Kne.n())
    for (_i = 0; _i < _test_master.size(); _i++)
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);

  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      Knn(_i, _j) += computeQpOffDiagJacobian(Moose::MasterMaster, jvar);
}
