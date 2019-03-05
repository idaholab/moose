//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMaterial.h"
#include "Function.h"

registerADMooseObject("NavierStokesApp", INSADMaterial);

defineADValidParams(
    INSADMaterial,
    ADMaterial,
    params.addClassDescription("This is the material class used to compute some of the strong "
                               "residuals for the INS equations.");
    params.addRequiredCoupledVar("velocity", "The velocity");
    params.addRequiredCoupledVar("pressure", "The pressure");
    params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
    params.addParam<bool>("transient_term",
                          true,
                          "Whether there should be a transient term in the momentum residuals.");
    params.addParam<bool>("integrate_p_by_parts",
                          true,
                          "Whether to integrate the pressure by parts");
    params.addParam<bool>("include_viscous_term_in_strong_form",
                          false,
                          "Whether to include the strong form of the viscous term in the momentum "
                          "equation strong residual. The method is more consistent if set to true, "
                          "but it incurs quite a bit more computational expense");
    params.addParam<RealVectorValue>("gravity", "Direction of the gravity vector");
    params.addParam<FunctionName>("function_x", 0, "The x-velocity mms forcing function.");
    params.addParam<FunctionName>("function_y", 0, "The y-velocity mms forcing function.");
    params.addParam<FunctionName>("function_z", 0, "The z-velocity mms forcing function."););

template <ComputeStage compute_stage>
INSADMaterial<compute_stage>::INSADMaterial(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _velocity(adCoupledVectorValue("velocity")),
    _grad_velocity(adCoupledVectorGradient("velocity")),
    _grad_p(adCoupledGradient("pressure")),
    _mu(adGetADMaterialProperty<Real>("mu_name")),
    _rho(adGetADMaterialProperty<Real>("rho_name")),
    _transient_term(adGetParam<bool>("transient_term")),
    _velocity_dot(_transient_term ? &adCoupledVectorDot("velocity") : nullptr),
    _integrate_p_by_parts(adGetParam<bool>("integrate_p_by_parts")),
    _include_viscous_term_in_strong_form(adGetParam<bool>("include_viscous_term_in_strong_form")),
    _mass_strong_residual(adDeclareADProperty<Real>("mass_strong_residual")),
    _convective_strong_residual(adDeclareADProperty<RealVectorValue>("convective_strong_residual")),
    _td_strong_residual(adDeclareADProperty<RealVectorValue>("td_strong_residual")),
    _gravity_strong_residual(adDeclareADProperty<RealVectorValue>("gravity_strong_residual")),
    _mms_function_strong_residual(
        adDeclareProperty<RealVectorValue>("mms_function_strong_residual")),
    _momentum_strong_residual(adDeclareADProperty<RealVectorValue>("momentum_strong_residual")),
    _x_vel_fn(getFunction("function_x")),
    _y_vel_fn(getFunction("function_y")),
    _z_vel_fn(getFunction("function_z"))
{
  if (parameters.isParamSetByUser("gravity"))
  {
    _gravity_set = true;
    _gravity = adGetParam<RealVectorValue>("gravity");
  }
  else
    _gravity_set = false;
  if (adGetParam<bool>("include_viscous_term_in_strong_form"))
    mooseError("Sorry no TypeNTensor operations are currently implemented, so we cannot add the "
               "strong form contribution of the viscous term. Note that for linear elements, this "
               "introduces no error, and in general for bi-linear elements, the error is small");
}

template <ComputeStage compute_stage>
void
INSADMaterial<compute_stage>::computeQpProperties()
{
  _mass_strong_residual[_qp] = -_grad_velocity[_qp].tr();
  _convective_strong_residual[_qp] = _rho[_qp] * _grad_velocity[_qp] * _velocity[_qp];
  _td_strong_residual[_qp] =
      _transient_term ? _rho[_qp] * (*_velocity_dot)[_qp] : ADRealVectorValue(0);
  _gravity_strong_residual[_qp] = _gravity_set ? _rho[_qp] * _gravity : ADRealVectorValue(0);
  _mms_function_strong_residual[_qp] = -RealVectorValue(_x_vel_fn.value(_t, _q_point[_qp]),
                                                        _y_vel_fn.value(_t, _q_point[_qp]),
                                                        _z_vel_fn.value(_t, _q_point[_qp]));
  _momentum_strong_residual[_qp] = _gravity_strong_residual[_qp] +
                                   _mms_function_strong_residual[_qp] +
                                   _convective_strong_residual[_qp] + _td_strong_residual[_qp];
}
