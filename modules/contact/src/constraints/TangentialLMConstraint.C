//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialLMConstraint.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"

#include "libmesh/libmesh.h" // uint

registerMooseObject("ContactApp", TangentialLMConstraint);

template <>
InputParameters
validParams<TangentialLMConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;

  params.addCoupledVar(
      "contact_pressure",
      "The contact pressure. If using LM, this should be the normal lagrange multiplier");
  params.addCoupledVar("vel_y", "The y velocity");
  params.addCoupledVar("vel_z", "The z velocity");
  params.addRequiredParam<Real>("mu", "The coefficient of friction.");
  params.addParam<Real>("lambda", .95, "The weighting coefficient from Chen");

  return params;
}

template <typename T>
int
sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}

TangentialLMConstraint::TangentialLMConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _contact_pressure(getVar("contact_pressure", 0)->dofValues()),
    _contact_pressure_id(coupled("contact_pressure")),

    _vel_x(_master_var.dofValues()),
    _vel_x_id(_master_var_num),

    _vel_y(getVar("vel_y", 0)->dofValues()),
    _vel_y_id(coupled("vel_y")),

    _vel_z(isCoupled("vel_z") ? coupledValue("vel_z") : _zero),
    _vel_z_id(isCoupled("vel_z") ? coupled("vel_z") : libMesh::invalid_uint),

    _mu(getParam<Real>("mu")),
    _lambda(getParam<Real>("lambda")),
    _epsilon(std::numeric_limits<Real>::epsilon())
{
  _overwrite_slave_residual = false;
}

Real
TangentialLMConstraint::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

void
TangentialLMConstraint::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  _qp = 0;

  re(0) = computeQpResidual(Moose::Slave);
}

void
TangentialLMConstraint::computeJacobian()
{
  _Kee.resize(1, 1);
  _connected_dof_indices.clear();
  _connected_dof_indices.push_back(_var.nodalDofIndex());

  _qp = 0;

  _Kee(0, 0) += computeQpJacobian(Moose::SlaveSlave);
}

void
TangentialLMConstraint::computeOffDiagJacobian(unsigned jvar)
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
}

Real TangentialLMConstraint::computeQpResidual(Moose::ConstraintType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure[_qp] < _epsilon)
        return _u_slave[_qp];
      else
      {
        RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);
        Real v_dot_tan = RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * tangent_vec;
        Real a = std::abs(_u_slave[_qp]) < _epsilon
                     ? -v_dot_tan
                     : -v_dot_tan * std::abs(_u_slave[_qp]) / _u_slave[_qp];
        Real b = _mu * _contact_pressure[_qp] - std::abs(_u_slave[_qp]);
        return _lambda * (a + b - std::sqrt(a * a + b * b)) +
               (1. - _lambda) * std::max(0., a) * std::max(0., b);
      }
    }
  }
  return 0;
}

Real TangentialLMConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure[_qp] < _epsilon)
        return 1.;
      else
      {
        RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);
        Real v_dot_tan = RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * tangent_vec;
        Real a = std::abs(_u_slave[_qp]) < _epsilon
                     ? -v_dot_tan
                     : -v_dot_tan * std::abs(_u_slave[_qp]) / _u_slave[_qp];

        Real b = _mu * _contact_pressure[_qp] - std::abs(_u_slave[_qp]);
        Real db_duj = -(_u_slave[_qp] >= 0 ? 1. : -1.);

        if (std::abs(a) < _epsilon)
          return _lambda * (1. - (b > 0 ? 1. : -1.)) * db_duj;

        else
          return _lambda * (1. - b / std::sqrt(a * a + b * b)) * db_duj +
                 (1. - _lambda) * std::max(0., a) * (b > 0 ? 1. : 0.) * db_duj;
      }
    }
  }
  return 0;
}

Real
TangentialLMConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                                 unsigned jvar)
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      if (_contact_pressure[_qp] < _epsilon)
        return 0.;

      RealVectorValue tangent_vec(pinfo->_normal(1), -pinfo->_normal(0), 0);
      Real v_dot_tan = RealVectorValue(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]) * tangent_vec;
      Real a = std::abs(_u_slave[_qp]) < _epsilon
                   ? -v_dot_tan
                   : -v_dot_tan * std::abs(_u_slave[_qp]) / _u_slave[_qp];
      Real b = _mu * _contact_pressure[_qp] - std::abs(_u_slave[_qp]);
      if (jvar == _contact_pressure_id)
      {
        Real db_dlmj = _mu;
        if (std::abs(a) < _epsilon)
          return _lambda * (1. - (b >= 0 ? 1. : -1.)) * db_dlmj;
        else
          return _lambda * (1. - b / std::sqrt(a * a + b * b)) * db_dlmj +
                 (1. - _lambda) * std::max(0., a) * (b >= 0 ? 1. : 0.) * db_dlmj;
      }
      else if (jvar == _vel_x_id)
      {
        Real da_dvxj = std::abs(_u_slave[_qp]) < _epsilon ? -pinfo->_normal(1)
                                                          : -pinfo->_normal(1) * sgn(_u_slave[_qp]);
        if (std::abs(b) < _epsilon)
          return _lambda * (1. - sgn(a)) * da_dvxj;
        else
          return _lambda * (1. - a / std::sqrt(a * a + b * b)) * da_dvxj +
                 (1. - _lambda) * std::max(0., b) * (a > 0 ? 1. : 0.) * da_dvxj;
      }
      else if (jvar == _vel_y_id)
      {
        Real da_dvyj = std::abs(_u_slave[_qp]) < _epsilon ? pinfo->_normal(0)
                                                          : pinfo->_normal(0) * sgn(_u_slave[_qp]);
        if (std::abs(b) < _epsilon)
          return _lambda * (1. - sgn(a)) * da_dvyj;
        else
          return _lambda * (1. - a / std::sqrt(a * a + b * b)) * da_dvyj +
                 (1. - _lambda) * std::max(0., b) * (a > 0 ? 1. : 0.) * da_dvyj;
      }
    }
  }
  return 0.;
}
