/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SingleGrainRigidBodyMotion.h"
#include "GrainTrackerInterface.h"

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
  unsigned int grain_index = _grain_tracker.getOpToGrainsVector(_current_elem->id())[_op_index];
  if (grain_index != libMesh::invalid_uint)
  {
    const auto volume = _grain_tracker.getGrainVolume(grain_index);
    const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
    const RealGradient velocity_advection = _mt / volume * _grain_forces[grain_index] * _u[_qp]
                                            + _mr / volume * _grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _u[_qp];
    const Real div_velocity_advection = _mt / volume * _grain_forces[grain_index] * _grad_u[_qp]
                                        + _mr / volume * _grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _grad_u[_qp];

    return velocity_advection * _grad_u[_qp] * _test[_i][_qp] + div_velocity_advection * _u[_qp] * _test[_i][_qp];
  }

  return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpJacobian()
{
  unsigned int grain_index = _grain_tracker.getOpToGrainsVector(_current_elem->id())[_op_index];
  if (grain_index != libMesh::invalid_uint)
  {
    const auto volume = _grain_tracker.getGrainVolume(grain_index);
    const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
    getUserObjectEtaJacobians(_var_dofs[_j], _op_index, grain_index);

    const RealGradient velocity_advection = _mt / volume * _grain_forces[grain_index] * _u[_qp]
                                            + _mr / volume * _grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _u[_qp];
    const Real div_velocity_advection = _mt / volume * _grain_forces[grain_index] * _grad_u[_qp]
                                        + _mr / volume * _grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _grad_u[_qp];
    const RealGradient velocity_advection_derivative_eta = _mt / volume * (_grain_forces[grain_index] * _phi[_j][_qp] + _u[_qp] * _force_eta_jacobian)
                                                           + _mr / volume * ((_grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _phi[_j][_qp])
                                                           + (_torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _u[_qp]));
    const Real div_velocity_advection_derivative_eta = _mt / volume * (_grain_forces[grain_index] * _grad_phi[_j][_qp] + _force_eta_jacobian * _grad_u[_qp])
                                                       + _mr / volume * ((_grain_torques[grain_index].cross(_q_point[_qp] - centroid) * _grad_phi[_j][_qp])
                                                       + (_torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _grad_u[_qp]));

    return (velocity_advection * _grad_phi[_j][_qp] + div_velocity_advection * _phi[_j][_qp]) * _test[_i][_qp]
           + (velocity_advection_derivative_eta * _grad_u[_qp] + div_velocity_advection_derivative_eta * _u[_qp]) * _test[_i][_qp];
  }

  return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned int grain_index = _grain_tracker.getOpToGrainsVector(_current_elem->id())[_op_index];
  if (grain_index != libMesh::invalid_uint)
  {
    if (jvar == _c_var)
    {
      const auto volume = _grain_tracker.getGrainVolume(grain_index);
      const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
      getUserObjectCJacobians(_c_dofs[_j], grain_index);

      const RealGradient velocity_advection_derivative_c = _mt / volume * _force_c_jacobian * _u[_qp]
                                                            + _mr / volume * (_torque_c_jacobian.cross(_q_point[_qp] - centroid)) * _u[_qp];
      const Real div_velocity_advection_derivative_c = _mt / volume * _grad_u[_qp] * _force_c_jacobian
                                                        + _mr / volume * (_torque_c_jacobian.cross(_q_point[_qp] - centroid)) * _grad_u[_qp];

      return velocity_advection_derivative_c * _grad_u[_qp] * _test[_i][_qp]
             + div_velocity_advection_derivative_c * _u[_qp] * _test[_i][_qp];
    }

    for (unsigned int op = 0; op < _op_num; ++op)
      if (jvar == _vals_var[op])
      {
        const auto volume = _grain_tracker.getGrainVolume(grain_index);
        const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
        MooseVariable & jv = _sys.getVariable(_tid, _vals_var[op]);
        getUserObjectEtaJacobians(jv.dofIndices()[_j], op, grain_index);

        const RealGradient velocity_advection_derivative_eta = _mt / volume * _force_eta_jacobian * _u[_qp]
                                                               + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _u[_qp];
        const Real div_velocity_advection_derivative_eta = _mt / volume * _force_eta_jacobian * _grad_u[_qp]
                                                           + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _grad_u[_qp];

        return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
               + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
     }
   }

  return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpNonlocalJacobian(dof_id_type dof_index)
{
  unsigned int grain_index = _grain_tracker.getOpToGrainsVector(_current_elem->id())[_op_index];
  if (grain_index != libMesh::invalid_uint)
  {
    const auto volume = _grain_tracker.getGrainVolume(grain_index);
    const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
    getUserObjectEtaJacobians(dof_index, _op_index, grain_index);

    const RealGradient velocity_advection_derivative_eta = _mt / volume * _u[_qp] * _force_eta_jacobian
                                                           + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _u[_qp];
    const Real div_velocity_advection_derivative_eta = _mt / volume * _force_eta_jacobian * _grad_u[_qp]
                                                       + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _grad_u[_qp];

    return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
           + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
   }

   return 0.0;
}

Real
SingleGrainRigidBodyMotion::computeQpNonlocalOffDiagJacobian(unsigned int jvar, dof_id_type dof_index)
{
  unsigned int grain_index = _grain_tracker.getOpToGrainsVector(_current_elem->id())[_op_index];
  if (grain_index != libMesh::invalid_uint)
  {
    if (jvar == _c_var)
    {
      const auto volume = _grain_tracker.getGrainVolume(grain_index);
      const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
      getUserObjectCJacobians(dof_index, grain_index);

      const RealGradient velocity_advection_derivative_c = _mt / volume * _u[_qp] * _force_c_jacobian
                                                            + _mr / volume * _torque_c_jacobian.cross(_q_point[_qp] - centroid) * _u[_qp];
      const Real div_velocity_advection_derivative_c = _mt / volume * _grad_u[_qp] * _force_c_jacobian
                                                       + _mr / volume * _torque_c_jacobian.cross(_q_point[_qp] - centroid) * _grad_u[_qp];

      return velocity_advection_derivative_c * _grad_u[_qp] * _test[_i][_qp]
             + div_velocity_advection_derivative_c * _u[_qp] * _test[_i][_qp];
    }

    for (unsigned int op = 0; op < _op_num; ++op)
      if (jvar == _vals_var[op])
      {
        const auto volume = _grain_tracker.getGrainVolume(grain_index);
        const auto centroid = _grain_tracker.getGrainCentroid(grain_index);
        getUserObjectEtaJacobians(dof_index, op, grain_index);

        const RealGradient velocity_advection_derivative_eta = _mt / volume * _force_eta_jacobian * _u[_qp]
                                                               + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _u[_qp];
        const Real div_velocity_advection_derivative_eta = _mt / volume * _force_eta_jacobian * _grad_u[_qp]
                                                           + _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid) * _grad_u[_qp];

        return velocity_advection_derivative_eta * _grad_u[_qp] * _test[_i][_qp]
               + div_velocity_advection_derivative_eta * _u[_qp] * _test[_i][_qp];
      }
  }

  return 0.0;
}
