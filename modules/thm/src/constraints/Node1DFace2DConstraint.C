#include "Node1DFace2DConstraint.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariable.h"

template <>
InputParameters
validParams<Node1DFace2DConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();

  return params;
}

Node1DFace2DConstraint::Node1DFace2DConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters), _JxW(_assembly.JxWNeighbor())
{
}

Real
Node1DFace2DConstraint::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

void
Node1DFace2DConstraint::computeJacobian()
{
  // diagonal part
  _qp = 0;

  getConnectedDofIndices(_var.number());
  _phi_slave.resize(_connected_dof_indices.size());
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);
    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }
  std::vector<dof_id_type> slave_dof_indices(1, _var.nodalDofIndex());

  // Master side
  _Jnn.resize(_test_master.size(), _phi_master.size());
  _Jnn.zero();
  // dmaster/dmaster
  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      _Jnn(_i, _j) += computeQpJacobian(Moose::MasterMaster);
  _assembly.cacheJacobianBlock(_Jnn,
                               _master_var.dofIndicesNeighbor(),
                               _master_var.dofIndicesNeighbor(),
                               _master_var.scalingFactor());

  // dmaster/dslave
  _Jne.resize(_test_master.size(), _connected_dof_indices.size());
  _Jne.zero();
  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_slave.size(); _j++)
      _Jne(_i, _j) += computeQpJacobian(Moose::MasterSlave);
  _assembly.cacheJacobianBlock(
      _Jne, _master_var.dofIndicesNeighbor(), _connected_dof_indices, _master_var.scalingFactor());

  // dslave/dslave
  _Jee.resize(_phi_slave.size(), _test_slave.size());
  _Jee.zero();
  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_slave.size(); _j++)
      _Jee(_j, _i) += computeQpJacobian(Moose::SlaveSlave);
  _assembly.cacheJacobianBlock(
      _Jee, _connected_dof_indices, slave_dof_indices, _var.scalingFactor());

  // dslave/dmaster
  _Jen.resize(_test_slave.size(), _phi_master.size());
  _Jen.zero();
  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_master.size(); _j++)
      _Jen(_i, _j) += computeQpJacobian(Moose::SlaveMaster);
  _assembly.cacheJacobianBlock(
      _Jen, slave_dof_indices, _master_var.dofIndicesNeighbor(), _var.scalingFactor());
}

void
Node1DFace2DConstraint::computeOffDiagJacobian(unsigned int jvar)
{
  // dmaster/dslave
  MooseVariable & var = _sys.getFieldVariable<Real>(0, jvar);

  getConnectedDofIndices(var.number());
  _phi_slave.resize(_connected_dof_indices.size());
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if (_connected_dof_indices[j] == var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  _Jne.resize(_test_master.size(), _connected_dof_indices.size());
  _Jne.zero();
  for (_i = 0; _i < _test_master.size(); _i++)
    for (_j = 0; _j < _phi_slave.size(); _j++)
      _Jne(_i, _j) += computeQpOffDiagJacobian(Moose::MasterSlave, jvar);
  _assembly.cacheJacobianBlock(
      _Jne, _master_var.dofIndicesNeighbor(), _connected_dof_indices, _master_var.scalingFactor());

  // dslave/dslave
  getConnectedDofIndices(_var.number());
  _phi_slave.resize(_connected_dof_indices.size());
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);
    if (_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  std::vector<dof_id_type> slave_dof_indices(1, var.nodalDofIndex());

  _Jee.resize(_phi_slave.size(), _test_slave.size());
  _Jee.zero();
  for (_i = 0; _i < _test_slave.size(); _i++)
    for (_j = 0; _j < _phi_slave.size(); _j++)
      _Jee(_j, _i) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);
  _assembly.cacheJacobianBlock(
      _Jee, _connected_dof_indices, slave_dof_indices, _var.scalingFactor());
}

bool
Node1DFace2DConstraint::overwriteSlaveResidual()
{
  return false;
}
