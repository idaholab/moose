/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SingleGrainRigidBodyMotion.h"

template<>
InputParameters validParams<SingleGrainRigidBodyMotion>()
{
  InputParameters params = validParams<GrainRigidBodyMotionBase>();
  params.addClassDescription("Adds rigid mody motion to a single grain");
  params.addParam<unsigned int>("op_index", 0, "Grain number for the kernel to be applied");
  return params;
}

SingleGrainRigidBodyMotion::SingleGrainRigidBodyMotion(const InputParameters & parameters) :
    GrainRigidBodyMotionBase(parameters),
    _op_index(getParam<unsigned int>("op_index"))
{
}

Real
SingleGrainRigidBodyMotion::computeQpResidual()
{
  const RealGradient velocity_advection = _mt / _grain_volumes[_op_index] * _grain_forces[_op_index] * _u[_qp]
                                          + _mr / _grain_volumes[_op_index] * _grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
  const Real div_velocity_advection = _mt / _grain_volumes[_op_index] * _grain_forces[_op_index] * _grad_u[_qp]
                                      + _mr / _grain_volumes[_op_index] * _grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];

  return velocity_advection * _grad_u[_qp] * _test[_i][_qp] + div_velocity_advection * _u[_qp] * _test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpJacobian()
{
  RealGradient force_eta_jacobian;
  RealGradient torque_eta_jacobian;
  force_eta_jacobian(0) = _grain_force_eta_jacobians[_op_index][(6*_op_index+0)*_total_dofs+_var.dofIndices()[_j]];
  force_eta_jacobian(1) = _grain_force_eta_jacobians[_op_index][(6*_op_index+1)*_total_dofs+_var.dofIndices()[_j]];
  force_eta_jacobian(2) = _grain_force_eta_jacobians[_op_index][(6*_op_index+2)*_total_dofs+_var.dofIndices()[_j]];
  torque_eta_jacobian(0) = _grain_force_eta_jacobians[_op_index][(6*_op_index+3)*_total_dofs+_var.dofIndices()[_j]];
  torque_eta_jacobian(1) = _grain_force_eta_jacobians[_op_index][(6*_op_index+4)*_total_dofs+_var.dofIndices()[_j]];
  torque_eta_jacobian(2) = _grain_force_eta_jacobians[_op_index][(6*_op_index+5)*_total_dofs+_var.dofIndices()[_j]];

  const RealGradient velocity_advection = _mt / _grain_volumes[_op_index] * _grain_forces[_op_index] * _u[_qp]
                                          + _mr / _grain_volumes[_op_index] * _grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
  const Real div_velocity_advection = _mt / _grain_volumes[_op_index] * _grain_forces[_op_index] * _grad_u[_qp]
                                      + _mr / _grain_volumes[_op_index] * _grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];
  const RealGradient velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * (_grain_forces[_op_index] * _phi[_j][_qp] + _u[_qp] * force_eta_jacobian)
                                                         + _mr / _grain_volumes[_op_index] * ((_grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _phi[_j][_qp])
                                                         + (torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp]));
  const Real div_velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * (_grain_forces[_op_index] * _grad_phi[_j][_qp] + force_eta_jacobian * _grad_u[_qp])
                                                     + _mr / _grain_volumes[_op_index] * ((_grain_torques[_op_index].cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_phi[_j][_qp])
                                                     + (torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp]));

  return (velocity_advection * _grad_phi[_j][_qp] + div_velocity_advection * _phi[_j][_qp]) * _test[_i][_qp]
         + (velocity_advection_derivative_eta * _grad_u[_qp] + div_velocity_advection_derivative_eta * _u[_qp]) * _test[_i][_qp];
}

Real
SingleGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
  {
    MooseVariable & jv = _sys.getVariable(_tid, _c_var);
    RealGradient force_c_jacobian = 0.0;
    RealGradient torque_c_jacobian = 0.0;
    force_c_jacobian(0) = _grain_force_c_jacobians[(6*_op_index+0)*_total_dofs+jv.dofIndices()[_j]];
    force_c_jacobian(1) = _grain_force_c_jacobians[(6*_op_index+1)*_total_dofs+jv.dofIndices()[_j]];
    force_c_jacobian(2) = _grain_force_c_jacobians[(6*_op_index+2)*_total_dofs+jv.dofIndices()[_j]];
    torque_c_jacobian(0) = _grain_force_c_jacobians[(6*_op_index+3)*_total_dofs+jv.dofIndices()[_j]];
    torque_c_jacobian(1) = _grain_force_c_jacobians[(6*_op_index+4)*_total_dofs+jv.dofIndices()[_j]];
    torque_c_jacobian(2) = _grain_force_c_jacobians[(6*_op_index+5)*_total_dofs+jv.dofIndices()[_j]];

    const RealGradient velocity_advection_derivative_c = _mt / _grain_volumes[_op_index] * force_c_jacobian * _u[_qp]
                                                          + _mr / _grain_volumes[_op_index] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index])) * _u[_qp];
    const Real div_velocity_advection_derivative_c = _mt / _grain_volumes[_op_index] * _grad_u[_qp] * force_c_jacobian
                                                      + _mr / _grain_volumes[_op_index] * (torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index])) * _grad_u[_qp];

    return velocity_advection_derivative_c * _grad_u[_qp] * _test[_i][_qp]
           + div_velocity_advection_derivative_c * _u[_qp] * _test[_i][_qp];
  }

  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      MooseVariable & jv = _sys.getVariable(_tid, _vals_var[i]);
      RealGradient force_eta_jacobian = 0.0;
      RealGradient torque_eta_jacobian = 0.0;
      force_eta_jacobian(0) = _grain_force_eta_jacobians[i][(6*_op_index+0)*_total_dofs+jv.dofIndices()[_j]];
      force_eta_jacobian(1) = _grain_force_eta_jacobians[i][(6*_op_index+1)*_total_dofs+jv.dofIndices()[_j]];
      force_eta_jacobian(2) = _grain_force_eta_jacobians[i][(6*_op_index+2)*_total_dofs+jv.dofIndices()[_j]];
      torque_eta_jacobian(0) = _grain_force_eta_jacobians[i][(6*_op_index+3)*_total_dofs+jv.dofIndices()[_j]];
      torque_eta_jacobian(1) = _grain_force_eta_jacobians[i][(6*_op_index+4)*_total_dofs+jv.dofIndices()[_j]];
      torque_eta_jacobian(2) = _grain_force_eta_jacobians[i][(6*_op_index+5)*_total_dofs+jv.dofIndices()[_j]];

      const RealGradient velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * force_eta_jacobian * _u[_qp]
                                                             + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
      const Real div_velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * force_eta_jacobian * _grad_u[_qp]
                                                         + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];

      return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
             + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
    }

  return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpNonlocalJacobian(dof_id_type dof_index)
{
  RealGradient force_eta_jacobian;
  RealGradient torque_eta_jacobian;
  force_eta_jacobian(0) = _grain_force_eta_jacobians[_op_index][(6*_op_index+0)*_total_dofs+dof_index];
  force_eta_jacobian(1) = _grain_force_eta_jacobians[_op_index][(6*_op_index+1)*_total_dofs+dof_index];
  force_eta_jacobian(2) = _grain_force_eta_jacobians[_op_index][(6*_op_index+2)*_total_dofs+dof_index];
  torque_eta_jacobian(0) = _grain_force_eta_jacobians[_op_index][(6*_op_index+3)*_total_dofs+dof_index];
  torque_eta_jacobian(1) = _grain_force_eta_jacobians[_op_index][(6*_op_index+4)*_total_dofs+dof_index];
  torque_eta_jacobian(2) = _grain_force_eta_jacobians[_op_index][(6*_op_index+5)*_total_dofs+dof_index];

  const RealGradient velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * _u[_qp] * force_eta_jacobian
                                                         + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
  const Real div_velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * force_eta_jacobian * _grad_u[_qp]
                                                     + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];

  return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
         + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
}


Real
SingleGrainRigidBodyMotion::computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index)
{
  if (jvar == _c_var)
  {
    RealGradient force_c_jacobian = 0.0;
    RealGradient torque_c_jacobian = 0.0;
    force_c_jacobian(0) = _grain_force_c_jacobians[(6*_op_index+0)*_total_dofs+dof_index];
    force_c_jacobian(1) = _grain_force_c_jacobians[(6*_op_index+1)*_total_dofs+dof_index];
    force_c_jacobian(2) = _grain_force_c_jacobians[(6*_op_index+2)*_total_dofs+dof_index];
    torque_c_jacobian(0) = _grain_force_c_jacobians[(6*_op_index+3)*_total_dofs+dof_index];
    torque_c_jacobian(1) = _grain_force_c_jacobians[(6*_op_index+4)*_total_dofs+dof_index];
    torque_c_jacobian(2) = _grain_force_c_jacobians[(6*_op_index+5)*_total_dofs+dof_index];
    const RealGradient velocity_advection_derivative_c = _mt / _grain_volumes[_op_index] * _u[_qp] * force_c_jacobian
                                                          + _mr / _grain_volumes[_op_index] * torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
    const Real div_velocity_advection_derivative_c = _mt / _grain_volumes[_op_index] * _grad_u[_qp] * force_c_jacobian
                                                     + _mr / _grain_volumes[_op_index] * torque_c_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];

    return velocity_advection_derivative_c * _grad_u[_qp] * _test[_i][_qp]
           + div_velocity_advection_derivative_c * _u[_qp] * _test[_i][_qp];
  }

  for (unsigned int i=0; i<_op_num; ++i)
    if (jvar == _vals_var[i])
    {
      RealGradient force_eta_jacobian = 0.0;
      RealGradient torque_eta_jacobian = 0.0;
      force_eta_jacobian(0) = _grain_force_eta_jacobians[i][(6*_op_index+0)*_total_dofs+dof_index];
      force_eta_jacobian(1) = _grain_force_eta_jacobians[i][(6*_op_index+1)*_total_dofs+dof_index];
      force_eta_jacobian(2) = _grain_force_eta_jacobians[i][(6*_op_index+2)*_total_dofs+dof_index];
      torque_eta_jacobian(0) = _grain_force_eta_jacobians[i][(6*_op_index+3)*_total_dofs+dof_index];
      torque_eta_jacobian(1) = _grain_force_eta_jacobians[i][(6*_op_index+4)*_total_dofs+dof_index];
      torque_eta_jacobian(2) = _grain_force_eta_jacobians[i][(6*_op_index+5)*_total_dofs+dof_index];

      const RealGradient velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * force_eta_jacobian * _u[_qp]
                                                             + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _u[_qp];
      const Real div_velocity_advection_derivative_eta = _mt / _grain_volumes[_op_index] * force_eta_jacobian * _grad_u[_qp]
                                                         + _mr / _grain_volumes[_op_index] * torque_eta_jacobian.cross(_q_point[_qp] - _grain_centers[_op_index]) * _grad_u[_qp];

      return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
             + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
    }

  return 0.0;
}
