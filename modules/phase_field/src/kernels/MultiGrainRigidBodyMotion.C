/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiGrainRigidBodyMotion.h"

template<>
InputParameters validParams<MultiGrainRigidBodyMotion>()
{
  InputParameters params = validParams<GrainRigidBodyMotionBase>();
  params.addClassDescription("Adds rigid mody motion to grains");
  return params;
}

MultiGrainRigidBodyMotion::MultiGrainRigidBodyMotion(const InputParameters & parameters) :
    GrainRigidBodyMotionBase(parameters)
{
}

Real
MultiGrainRigidBodyMotion::computeQpResidual()
{
  RealGradient velocity_advection = 0.0;
  Real div_velocity_advection = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    velocity_advection += _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * _grain_forces[i])
                                            + _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_advection += _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * _grain_forces[i])
                                          + _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];
  }

  return velocity_advection * _grad_c[_qp] * _test[_i][_qp] + div_velocity_advection * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeQpJacobian()
{
  if (_var.number() == _c_var) //Requires c jacobian
    return computeCVarJacobianEntry(_var.dofIndices()[_j]);

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var) //Requires c jacobian
  {
    MooseVariable & jv = _sys.getVariable(_tid, _c_var);
    return computeCVarJacobianEntry(jv.dofIndices()[_j]);
  }

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      MooseVariable & jv = _sys.getVariable(_tid, _vals_var[i]);
      return computeEtaVarJacobianEntry(jv.dofIndices()[_j], i);
    }

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpNonlocalJacobian(dof_id_type dof_index)
{
  if (_var.number() == _c_var) //Requires c jacobian
    return computeCVarNonlocalJacobianEntry(dof_index);

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index)
{
  if (jvar == _c_var)
    return computeCVarNonlocalJacobianEntry(dof_index);

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
      return computeEtaVarNonlocalJacobianEntry(dof_index, i);

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeCVarJacobianEntry(dof_id_type jdof)
{
  RealGradient velocity_advection = 0.0;
  Real div_velocity_advection = 0.0;
  RealGradient velocity_advection_derivative_c = 0.0;
  Real div_velocity_advection_derivative_c = 0.0;
  RealGradient force_c_jacobian = 0.0;
  RealGradient torque_c_jacobian = 0.0;

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    force_c_jacobian(0) = _grain_force_c_jacobians[(6*i+0)*_total_dofs+jdof];
    force_c_jacobian(1) = _grain_force_c_jacobians[(6*i+1)*_total_dofs+jdof];
    force_c_jacobian(2) = _grain_force_c_jacobians[(6*i+2)*_total_dofs+jdof];
    torque_c_jacobian(0) = _grain_force_c_jacobians[(6*i+3)*_total_dofs+jdof];
    torque_c_jacobian(1) = _grain_force_c_jacobians[(6*i+4)*_total_dofs+jdof];
    torque_c_jacobian(2) = _grain_force_c_jacobians[(6*i+5)*_total_dofs+jdof];

    velocity_advection += _mt / _grain_volumes[i] * (*_vals[i])[_qp] * _grain_forces[i]
                          + _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_advection += _mt / _grain_volumes[i] * (*_grad_vals[i])[_qp] * _grain_forces[i]
                              + _mr / _grain_volumes[i] * (_grain_torques[i].cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];
    velocity_advection_derivative_c += _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * force_c_jacobian)
                                       + _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_advection_derivative_c += _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * force_c_jacobian)
                                           + _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];
  }

  return  velocity_advection * _grad_phi[_j][_qp] * _test[_i][_qp] + velocity_advection_derivative_c * _grad_c[_qp] * _test[_i][_qp]
          + div_velocity_advection * _phi[_j][_qp] * _test[_i][_qp] + div_velocity_advection_derivative_c * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeCVarNonlocalJacobianEntry(dof_id_type jdof)
{
  RealGradient velocity_advection_derivative_c = 0.0;
  Real div_velocity_advection_derivative_c = 0.0;
  RealGradient force_c_jacobian = 0.0;
  RealGradient torque_c_jacobian = 0.0;

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    force_c_jacobian(0) = _grain_force_c_jacobians[(6*i+0)*_total_dofs+jdof];
    force_c_jacobian(1) = _grain_force_c_jacobians[(6*i+1)*_total_dofs+jdof];
    force_c_jacobian(2) = _grain_force_c_jacobians[(6*i+2)*_total_dofs+jdof];
    torque_c_jacobian(0) = _grain_force_c_jacobians[(6*i+3)*_total_dofs+jdof];
    torque_c_jacobian(1) = _grain_force_c_jacobians[(6*i+4)*_total_dofs+jdof];
    torque_c_jacobian(2) = _grain_force_c_jacobians[(6*i+5)*_total_dofs+jdof];

    velocity_advection_derivative_c += _mt / _grain_volumes[i] * ((*_vals[i])[_qp] * force_c_jacobian)
                                       + _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_advection_derivative_c += _mt / _grain_volumes[i] * ((*_grad_vals[i])[_qp] * force_c_jacobian)
                                           + _mr / _grain_volumes[i] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];
  }
  return velocity_advection_derivative_c * _grad_c[_qp] * _test[_i][_qp] + div_velocity_advection_derivative_c * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeEtaVarJacobianEntry(dof_id_type jdof, unsigned int j)
{
  RealGradient velocity_advection_derivative_eta = 0.0;
  Real div_velocity_advection_derivative_eta = 0.0;
  RealGradient force_eta_jacobian;
  RealGradient torque_eta_jacobian;

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    force_eta_jacobian(0) = _grain_force_eta_jacobians[j][(6*i+0)*_total_dofs+jdof];
    force_eta_jacobian(1) = _grain_force_eta_jacobians[j][(6*i+1)*_total_dofs+jdof];
    force_eta_jacobian(2) = _grain_force_eta_jacobians[j][(6*i+2)*_total_dofs+jdof];
    torque_eta_jacobian(0) = _grain_force_eta_jacobians[j][(6*i+3)*_total_dofs+jdof];
    torque_eta_jacobian(1) = _grain_force_eta_jacobians[j][(6*i+4)*_total_dofs+jdof];
    torque_eta_jacobian(2) = _grain_force_eta_jacobians[j][(6*i+5)*_total_dofs+jdof];
    velocity_advection_derivative_eta += _mt / _grain_volumes[i] * (*_vals[i])[_qp] * force_eta_jacobian
                                         + _mr / _grain_volumes[i] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[i]) * (*_vals[i])[_qp];
    div_velocity_advection_derivative_eta += _mt / _grain_volumes[i] * (*_grad_vals[i])[_qp] * force_eta_jacobian
                                             + _mr / _grain_volumes[i] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[i]) * (*_grad_vals[i])[_qp];
    if (i == j)
    {
      velocity_advection_derivative_eta += _mt / _grain_volumes[i] * _grain_forces[i] * _phi[_j][_qp]
                                          + _mr / _grain_volumes[i] * _grain_torques[i].cross(_q_point[_qp] - _grain_centers[i]) * _phi[_j][_qp];
      div_velocity_advection_derivative_eta += _mt / _grain_volumes[i] * _grain_forces[i] * _grad_phi[_j][_qp]
                                            + _mr / _grain_volumes[i] * _grain_torques[i].cross(_q_point[_qp] - _grain_centers[i]) * _grad_phi[_j][_qp];
    }
  }

  return  velocity_advection_derivative_eta * _grad_c[_qp] * _test[_i][_qp] + div_velocity_advection_derivative_eta * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeEtaVarNonlocalJacobianEntry(dof_id_type jdof, unsigned int j)
{
  RealGradient velocity_advection_derivative_eta = 0.0;
  Real div_velocity_advection_derivative_eta = 0.0;
  RealGradient force_eta_jacobian = 0.0;
  RealGradient torque_eta_jacobian = 0.0;

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    force_eta_jacobian(0) = _grain_force_eta_jacobians[j][(6*i+0)*_total_dofs+jdof];
    force_eta_jacobian(1) = _grain_force_eta_jacobians[j][(6*i+1)*_total_dofs+jdof];
    force_eta_jacobian(2) = _grain_force_eta_jacobians[j][(6*i+2)*_total_dofs+jdof];
    torque_eta_jacobian(0) = _grain_force_eta_jacobians[j][(6*i+3)*_total_dofs+jdof];
    torque_eta_jacobian(1) = _grain_force_eta_jacobians[j][(6*i+4)*_total_dofs+jdof];
    torque_eta_jacobian(2) = _grain_force_eta_jacobians[j][(6*i+5)*_total_dofs+jdof];
    velocity_advection_derivative_eta += _mt / _grain_volumes[i] * (*_vals[i])[_qp] * force_eta_jacobian
                                         + _mr / _grain_volumes[i] * (torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_vals[i])[_qp];
    div_velocity_advection_derivative_eta += _mt / _grain_volumes[i] * (*_grad_vals[i])[_qp] * force_eta_jacobian
                                             + _mr / _grain_volumes[i] * (torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[i])) * (*_grad_vals[i])[_qp];
  }

  return  velocity_advection_derivative_eta * _grad_c[_qp] * _test[_i][_qp] + div_velocity_advection_derivative_eta * _c[_qp] * _test[_i][_qp];
}
