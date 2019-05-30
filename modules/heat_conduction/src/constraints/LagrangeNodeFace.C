#include "LagrangeNodeFace.h"
#include "Assembly.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"

registerMooseObject("HeatConductionApp", LagrangeNodeFace);

template <>
InputParameters
validParams<LagrangeNodeFace>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.addClassDescription("");
  params.addParam<Real>("k", 1.0, "Gap conductance");
  return params;
}

LagrangeNodeFace::LagrangeNodeFace(const InputParameters & parameters)
  : NodeFaceConstraint(parameters), _temp_slave(_master_var.nodalValue()), _k(getParam<Real>("k"))
{
}

Real
LagrangeNodeFace::computeQpSlaveValue()
{
  return 0.0;
}

void
LagrangeNodeFace::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  re(0) = computeQpResidual(Moose::Slave);
}

void
LagrangeNodeFace::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SlaveSlave);
}

void
LagrangeNodeFace::computeOffDiagJacobian(unsigned jvar)
{
  if (jvar == _var.number())
  {
    computeJacobian();
    return;
  }

  MooseVariableFEBase & var = _sys.getVariable(0, jvar);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(var.nodalDofIndex());

  _qp = 0;

  _Kee.resize(1, 1);
  _Kee(0, 0) += computeQpOffDiagJacobian(Moose::SlaveSlave, jvar);

  DenseMatrix<Number> & Ken =
      _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar);

  auto master_j_size = var.dofIndicesNeighbor().size();
  for (_j = 0; _j < master_j_size; ++_j)
    Ken(0, _j) += computeQpOffDiagJacobian(Moose::SlaveMaster, jvar);
}

Real
LagrangeNodeFace::computeQpResidual(Moose::ConstraintType type)
{
  if (type != Moose::Slave)
    return 0.0;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      auto l = -pinfo->_distance;
      return (_k * (_u_master[_qp] - _temp_slave) / l - /*lm*/ _u_slave[_qp]);
    }
  }

  return 0.0;
}

Real
LagrangeNodeFace::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::MasterMaster:
      return 0.0;
    case Moose::SlaveMaster:
      return 0.0;
    case Moose::MasterSlave:
      return 0.0;
    case Moose::SlaveSlave:
      return 0.0;
    default:
      return 0.0;
  }
}
