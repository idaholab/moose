//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainRigidBodyMotionBase.h"

// MOOSE includes
#include "GrainTrackerInterface.h"
#include "MooseVariable.h"

InputParameters
GrainRigidBodyMotionBase::validParams()
{
  InputParameters params = NonlocalKernel::validParams();
  params.addClassDescription("Base class for adding rigid body motion to grains");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variable names");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "type of force density under consideration");
  params.addParam<Real>(
      "translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addRequiredParam<UserObjectName>(
      "grain_force", "UserObject for getting force and torque acting on grains");
  params.addRequiredParam<UserObjectName>("grain_tracker_object",
                                          "The FeatureFloodCount UserObject to get values from.");
  params.addRequiredParam<VectorPostprocessorName>("grain_volumes",
                                                   "The feature volume VectorPostprocessorValue.");
  return params;
}

GrainRigidBodyMotionBase::GrainRigidBodyMotionBase(const InputParameters & parameters)
  : NonlocalKernel(parameters),
    _var_dofs(_var.dofIndices()),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_dofs(getVar("c", 0)->dofIndices()),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _vals_var(coupledIndices("v")),
    _grad_vals(coupledGradients("v")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _grain_force_c_jacobians(_grain_force_torque.getForceCJacobians()),
    _grain_force_eta_jacobians(_grain_force_torque.getForceEtaJacobians()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker_object")),
    _grain_volumes(getVectorPostprocessorValue("grain_volumes", "feature_volumes"))
{
}

void
GrainRigidBodyMotionBase::timestepSetup()
{
  _total_dofs = _subproblem.es().n_dofs();
}

bool
GrainRigidBodyMotionBase::globalDoFEnabled(MooseVariableFEBase & /*var*/, dof_id_type /*dof_index*/)
{
  if (_velocity_advection_jacobian(0) == 0 && _velocity_advection_jacobian(1) == 0 &&
      _velocity_advection_jacobian(2) == 0)
    return false;

  return true;
}

void
GrainRigidBodyMotionBase::precalculateResidual()
{
  calculateAdvectionVelocity();
}

void
GrainRigidBodyMotionBase::precalculateJacobian()
{
  calculateAdvectionVelocity();
}

void
GrainRigidBodyMotionBase::precalculateOffDiagJacobian(unsigned int /* jvar */)
{
  calculateAdvectionVelocity();
}
