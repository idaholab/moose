//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiGrainRigidBodyMotion.h"

// MOOSE includes
#include "GrainTrackerInterface.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", MultiGrainRigidBodyMotion);

InputParameters
MultiGrainRigidBodyMotion::validParams()
{
  InputParameters params = GrainRigidBodyMotionBase::validParams();
  params.addClassDescription("Adds rigid body motion to grains");
  return params;
}

MultiGrainRigidBodyMotion::MultiGrainRigidBodyMotion(const InputParameters & parameters)
  : GrainRigidBodyMotionBase(parameters)
{
}

Real
MultiGrainRigidBodyMotion::computeQpResidual()
{
  return _velocity_advection * _grad_c[_qp] * _test[_i][_qp];
}

Real
MultiGrainRigidBodyMotion::computeQpJacobian()
{
  if (_var.number() == _c_var) // Requires c jacobian
    return _velocity_advection * _grad_phi[_j][_qp] * _test[_i][_qp] +
           _velocity_advection_jacobian * _grad_c[_qp] * _test[_i][_qp];

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var) // Requires c jacobian
    return _velocity_advection * _grad_phi[_j][_qp] * _test[_i][_qp] +
           _velocity_advection_jacobian * _grad_c[_qp] * _test[_i][_qp];
  else
    return _velocity_advection_jacobian * _grad_c[_qp] * _test[_i][_qp];
}

Real MultiGrainRigidBodyMotion::computeQpNonlocalJacobian(dof_id_type /*dof_index*/)
{
  if (_var.number() == _c_var) // Requires c jacobian
    return _velocity_advection_jacobian * _grad_c[_qp] * _test[_i][_qp];

  return 0.0;
}

Real
MultiGrainRigidBodyMotion::computeQpNonlocalOffDiagJacobian(unsigned int /* jvar */,
                                                            dof_id_type /* dof_index */)
{
  return _velocity_advection_jacobian * _grad_c[_qp] * _test[_i][_qp];
}

void
MultiGrainRigidBodyMotion::getUserObjectJacobian(unsigned int jvar, dof_id_type dof_index)
{
  _velocity_advection_jacobian = 0.0;

  for (MooseIndex(_grain_ids) i = 0; i < _grain_ids.size(); ++i)
  {
    auto grain_id = _grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      mooseAssert(grain_id < _grain_volumes.size(), "grain_id out of bounds");
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);
      RealGradient force_jacobian;
      RealGradient torque_jacobian;

      if (jvar == _c_var)
      {
        force_jacobian(0) = _grain_force_c_jacobians[(6 * grain_id + 0) * _total_dofs + dof_index];
        force_jacobian(1) = _grain_force_c_jacobians[(6 * grain_id + 1) * _total_dofs + dof_index];
        force_jacobian(2) = _grain_force_c_jacobians[(6 * grain_id + 2) * _total_dofs + dof_index];
        torque_jacobian(0) = _grain_force_c_jacobians[(6 * grain_id + 3) * _total_dofs + dof_index];
        torque_jacobian(1) = _grain_force_c_jacobians[(6 * grain_id + 4) * _total_dofs + dof_index];
        torque_jacobian(2) = _grain_force_c_jacobians[(6 * grain_id + 5) * _total_dofs + dof_index];
      }

      for (unsigned int jvar_index = 0; jvar_index < _op_num; ++jvar_index)
        if (jvar == _vals_var[jvar_index])
        {
          force_jacobian(0) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 0) * _total_dofs + dof_index];
          force_jacobian(1) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 1) * _total_dofs + dof_index];
          force_jacobian(2) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 2) * _total_dofs + dof_index];
          torque_jacobian(0) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 3) * _total_dofs + dof_index];
          torque_jacobian(1) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 4) * _total_dofs + dof_index];
          torque_jacobian(2) =
              _grain_force_eta_jacobians[jvar_index][(6 * grain_id + 5) * _total_dofs + dof_index];
        }

      const auto force_jac = _mt / volume * force_jacobian;
      const auto torque_jac =
          _mr / volume * torque_jacobian.cross(_current_elem->vertex_average() - centroid);

      _velocity_advection_jacobian += (force_jac + torque_jac);
    }
  }
}

void
MultiGrainRigidBodyMotion::calculateAdvectionVelocity()
{
  _velocity_advection = 0.0;
  _grain_ids = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  for (MooseIndex(_grain_ids) i = 0; i < _grain_ids.size(); ++i)
  {
    auto grain_id = _grain_ids[i];
    if (grain_id != FeatureFloodCount::invalid_id)
    {
      mooseAssert(grain_id < _grain_volumes.size(), "grain_id out of bounds");
      const auto volume = _grain_volumes[grain_id];
      const auto centroid = _grain_tracker.getGrainCentroid(grain_id);
      const auto force = _mt / volume * _grain_forces[grain_id];
      const auto torque =
          _mr / volume *
          (_grain_torques[grain_id].cross(_current_elem->vertex_average() - centroid));

      _velocity_advection += (force + torque);
    }
  }
}
