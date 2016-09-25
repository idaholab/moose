/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainRigidBodyMotionBase.h"
#include "GrainTrackerInterface.h"

template<>
InputParameters validParams<GrainRigidBodyMotionBase>()
{
  InputParameters params = validParams<NonlocalKernel>();
  params.addClassDescription("Base class for adding rigid body motion to grains");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variable names");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define type of force density under consideration");
  params.addParam<Real>("translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addRequiredParam<UserObjectName>("grain_force", "UserObject for getting force and torque acting on grains");
  params.addRequiredParam<UserObjectName>("grain_tracker_object", "The FeatureFloodCount UserObject to get values from.");
  params.addRequiredParam<VectorPostprocessorName>("grain_volumes", "The feature volume VectorPostprocessorValue.");
  return params;
}

GrainRigidBodyMotionBase::GrainRigidBodyMotionBase(const InputParameters & parameters) :
    NonlocalKernel(parameters),
    _var_dofs(_var.dofIndices()),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_dofs(getVar("c", 0)->dofIndices()),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _vals_var(_op_num),
    _grad_vals(_op_num),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _grain_force_c_jacobians(_grain_force_torque.getForceCJacobians()),
    _grain_force_eta_jacobians(_grain_force_torque.getForceEtaJacobians()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "grain_volumes"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
    _grad_vals[i] = &coupledGradient("v",i);
  }
}

void
GrainRigidBodyMotionBase::timestepSetup()
{
  _total_dofs = _subproblem.es().n_dofs();
}

void
GrainRigidBodyMotionBase::getUserObjectCJacobians(dof_id_type dof_index, unsigned int grain_index)
{
  _force_c_jacobian(0) = _grain_force_c_jacobians[(6*grain_index+0)*_total_dofs+dof_index];
  _force_c_jacobian(1) = _grain_force_c_jacobians[(6*grain_index+1)*_total_dofs+dof_index];
  _force_c_jacobian(2) = _grain_force_c_jacobians[(6*grain_index+2)*_total_dofs+dof_index];
  _torque_c_jacobian(0) = _grain_force_c_jacobians[(6*grain_index+3)*_total_dofs+dof_index];
  _torque_c_jacobian(1) = _grain_force_c_jacobians[(6*grain_index+4)*_total_dofs+dof_index];
  _torque_c_jacobian(2) = _grain_force_c_jacobians[(6*grain_index+5)*_total_dofs+dof_index];
}

void
GrainRigidBodyMotionBase::getUserObjectEtaJacobians(dof_id_type dof_index, unsigned int jvar_index, unsigned int grain_index)
{
  _force_eta_jacobian(0) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+0)*_total_dofs+dof_index];
  _force_eta_jacobian(1) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+1)*_total_dofs+dof_index];
  _force_eta_jacobian(2) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+2)*_total_dofs+dof_index];
  _torque_eta_jacobian(0) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+3)*_total_dofs+dof_index];
  _torque_eta_jacobian(1) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+4)*_total_dofs+dof_index];
  _torque_eta_jacobian(2) = _grain_force_eta_jacobians[jvar_index][(6*grain_index+5)*_total_dofs+dof_index];
}
