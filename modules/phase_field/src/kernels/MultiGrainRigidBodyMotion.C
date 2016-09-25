/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiGrainRigidBodyMotion.h"
#include "GrainTrackerInterface.h"

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
  const auto & grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (auto i = beginIndex(grain_ids); i < grain_ids.size(); ++i)
  {
    auto grain_id = grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      mooseAssert(grain_id < _grain_volumes.size(), "grain_id out of bounds");
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);
      const auto force = _mt / volume * _grain_forces[grain_id];
      const auto torque = _mr / volume * (_grain_torques[grain_id].cross(_q_point[_qp] - centroid));

      velocity_advection += (force + torque) * (*_vals[i])[_qp];
      div_velocity_advection += (force + torque) * (*_grad_vals[i])[_qp];
    }
  }

  return velocity_advection * _grad_c[_qp] * _test[_i][_qp] + div_velocity_advection * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeQpJacobian()
{
  if (_var.number() == _c_var) //Requires c jacobian
    return computeCVarJacobianEntry(_var_dofs[_j]);

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var) //Requires c jacobian
    return computeCVarJacobianEntry(_c_dofs[_j]);

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
  RealGradient velocity_advection_jacobian_c = 0.0;
  Real div_velocity_advection_jacobian_c = 0.0;
  const auto & grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (auto i = beginIndex(grain_ids); i < grain_ids.size(); ++i)
  {
    auto grain_id = grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);

      getUserObjectCJacobians(jdof, grain_id);
      const auto force = _mt / volume * _grain_forces[grain_id];
      const auto torque = _mr / volume * (_grain_torques[grain_id].cross(_q_point[_qp] - centroid));
      const auto force_jac = _mt / volume * _force_c_jacobian;
      const auto torque_jac = _mr / volume * _torque_c_jacobian.cross(_q_point[_qp] - centroid);

      velocity_advection += (force + torque) * (*_vals[i])[_qp];
      div_velocity_advection += (force + torque) * (*_grad_vals[i])[_qp];
      velocity_advection_jacobian_c += (force_jac + torque_jac) * (*_vals[i])[_qp];
      div_velocity_advection_jacobian_c += (force_jac + torque_jac) * (*_grad_vals[i])[_qp];
    }
  }

  return velocity_advection * _grad_phi[_j][_qp] * _test[_i][_qp] + velocity_advection_jacobian_c * _grad_c[_qp] * _test[_i][_qp]
    + div_velocity_advection * _phi[_j][_qp] * _test[_i][_qp] + div_velocity_advection_jacobian_c * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeCVarNonlocalJacobianEntry(dof_id_type jdof)
{
  RealGradient velocity_advection_jacobian_c = 0.0;
  Real div_velocity_advection_jacobian_c = 0.0;
  const auto & grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (auto i = beginIndex(grain_ids); i < grain_ids.size(); ++i)
  {
    auto grain_id = grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);

      getUserObjectCJacobians(jdof, grain_id);
      const auto force_jac = _mt / volume * _force_c_jacobian;
      const auto torque_jac = _mr / volume * _torque_c_jacobian.cross(_q_point[_qp] - centroid);

      velocity_advection_jacobian_c += (force_jac + torque_jac) * (*_vals[i])[_qp];
      div_velocity_advection_jacobian_c += (force_jac + torque_jac) * (*_grad_vals[i])[_qp];
    }
  }

  return velocity_advection_jacobian_c * _grad_c[_qp] * _test[_i][_qp]
    + div_velocity_advection_jacobian_c * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeEtaVarJacobianEntry(dof_id_type jdof, unsigned int jvar_index)
{
  RealGradient velocity_advection_jacobian_eta = 0.0;
  Real div_velocity_advection_jacobian_eta = 0.0;
  const auto & grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (auto i = beginIndex(grain_ids); i < grain_ids.size(); ++i)
  {
    auto grain_id = grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);

      getUserObjectEtaJacobians(jdof, jvar_index, grain_id);
      const auto force = _mt / volume * _grain_forces[grain_id];
      const auto torque = _mr / volume * (_grain_torques[grain_id].cross(_q_point[_qp] - centroid));
      const auto force_jac = _mt / volume * _force_eta_jacobian;
      const auto torque_jac = _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid);

      velocity_advection_jacobian_eta += (force_jac + torque_jac) * (*_vals[i])[_qp];
      div_velocity_advection_jacobian_eta += (force_jac + torque_jac) * (*_grad_vals[i])[_qp];

      if (i == jvar_index)
      {
        velocity_advection_jacobian_eta += (force + torque) * _phi[_j][_qp];
        div_velocity_advection_jacobian_eta += (force + torque) * _grad_phi[_j][_qp];
      }
    }
  }

  return velocity_advection_jacobian_eta * _grad_c[_qp] * _test[_i][_qp]
    + div_velocity_advection_jacobian_eta * _c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeEtaVarNonlocalJacobianEntry(dof_id_type jdof, unsigned int jvar_index)
{
  RealGradient eta_jacobian_sum = 0.0;
  RealGradient velocity_advection_jacobian_eta = 0.0;
  Real div_velocity_advection_jacobian_eta = 0.0;
  const auto & grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (auto i = beginIndex(grain_ids); i < grain_ids.size(); ++i)
  {
    auto grain_id = grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);

      getUserObjectEtaJacobians(jdof, jvar_index, grain_id);
      const auto force_jac = _mt / volume * _force_eta_jacobian;
      const auto torque_jac = _mr / volume * _torque_eta_jacobian.cross(_q_point[_qp] - centroid);

      velocity_advection_jacobian_eta += (force_jac + torque_jac) * (*_vals[i])[_qp];
      div_velocity_advection_jacobian_eta += (force_jac + torque_jac) * (*_grad_vals[i])[_qp];
    }
  }

  return velocity_advection_jacobian_eta * _grad_c[_qp] * _test[_i][_qp]
    + div_velocity_advection_jacobian_eta * _c[_qp] * _test[_i][_qp];
}
