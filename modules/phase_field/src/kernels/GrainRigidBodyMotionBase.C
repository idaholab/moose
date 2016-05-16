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
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Base class for adding rigid mody motion to grains");
  params.addRequiredCoupledVar("c", "Concentration");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variable names");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define type of force density under consideration");
  return params;
}

GrainRigidBodyMotionBase::GrainRigidBodyMotionBase(const InputParameters & parameters) :
    Kernel(parameters),
    _c_var(coupled("c")),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_name(getVar("c", 0)->name()),
    _ncrys(coupledComponents("v")),
    _vals(_ncrys),
    _vals_var(_ncrys),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _velocity_advection(getMaterialProperty<std::vector<RealGradient> >(_base_name + "advection_velocity")),
    _div_velocity_advection(getMaterialProperty<std::vector<Real> >(_base_name + "advection_velocity_divergence")),
    _velocity_advection_derivative_c(getMaterialPropertyByName<std::vector<RealGradient> >(propertyNameFirst(_base_name + "advection_velocity", _c_name))),
    _div_velocity_advection_derivative_c(getMaterialPropertyByName<std::vector<Real> >(propertyNameFirst(_base_name + "advection_velocity_divergence", _c_name))),
    _velocity_advection_derivative_eta(getMaterialPropertyByName<std::vector<RealGradient> >(propertyNameFirst(_base_name + "advection_velocity", "eta")))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}
