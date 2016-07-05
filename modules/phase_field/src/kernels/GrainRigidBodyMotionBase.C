/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GrainRigidBodyMotionBase.h"

template<>
InputParameters validParams<GrainRigidBodyMotionBase>()
{
  InputParameters params = validParams<NonlocalKernel>();
  params.addClassDescription("Base class for adding rigid mody motion to grains");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variable names");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define type of force density under consideration");
  params.addParam<Real>("translation_constant", 500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant", 1.0, "constant value characterizing grain rotation");
  params.addParam<UserObjectName>("grain_data", "userobject for getting volume and center of mass of grains");
  params.addParam<UserObjectName>("grain_force", "userobject for getting force and torque acting on grains");
  return params;
}

GrainRigidBodyMotionBase::GrainRigidBodyMotionBase(const InputParameters & parameters) :
    NonlocalKernel(parameters),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_name(getVar("c", 0)->name()),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _vals_var(_op_num),
    _grad_vals(_op_num),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
    _grain_volumes(_grain_data.getGrainVolumes()),
    _grain_centers(_grain_data.getGrainCenters()),
    _grain_force_torque(getUserObject<GrainForceAndTorqueInterface>("grain_force")),
    _grain_forces(_grain_force_torque.getForceValues()),
    _grain_torques(_grain_force_torque.getTorqueValues()),
    _grain_force_c_jacobians(_grain_force_torque.getForceCJacobians()),
    _grain_force_eta_jacobians(_grain_force_torque.getForceEtaJacobians()),
    _mt(getParam<Real>("translation_constant")),
    _mr(getParam<Real>("rotation_constant"))

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
