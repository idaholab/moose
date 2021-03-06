//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVMomentumFriction.h"

registerMooseObject("NavierStokesApp", PNSFVMomentumFriction);
registerMooseObjectRenamed("NavierStokesApp",
                           PINSFVMomentumFriction,
                           "07/01/2021 00:00",
                           PNSFVMomentumFriction);

InputParameters
PNSFVMomentumFriction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Computes a friction force term on fluid in porous media in the "
                             "Navier Stokes i-th momentum equation.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<MaterialPropertyName>("linear_coef_name",
                                        "Name of a linear friction coefficient material property.");
  params.addParam<MaterialPropertyName>(
      "quadratic_coef_name", "Name of a quadratic friction coefficient material property.");
  params.addParam<MaterialPropertyName>("Darcy_name",
                                        "Name of the Darcy coefficients material property.");
  params.addParam<MaterialPropertyName>("Forchheimer_name",
                                        "Name of the Forchheimer coefficients material property.");

  params.addParam<MaterialPropertyName>("momentum_name",
                                        "Name of the superficial momentum material property for "
                                        "the Darcy and Forchheimer friction terms.");
  params.addParam<Real>("rho", "Constant density to use with incompressible flow.");

  return params;
}

PNSFVMomentumFriction::PNSFVMomentumFriction(const InputParameters & params)
  : FVElementalKernel(params),
    _component(getParam<MooseEnum>("momentum_component")),
    _linear_friction_matprop(isParamValid("linear_coef_name")
                                 ? &getADMaterialProperty<Real>("linear_coef_name")
                                 : nullptr),
    _quadratic_friction_matprop(isParamValid("quadratic_coef_name")
                                    ? &getADMaterialProperty<Real>("quadratic_coef_name")
                                    : nullptr),
    _cL(isParamValid("Darcy_name") ? &getADMaterialProperty<RealVectorValue>("Darcy_name")
                                   : nullptr),
    _cQ(isParamValid("Forchheimer_name")
            ? &getADMaterialProperty<RealVectorValue>("Forchheimer_name")
            : nullptr),
    _use_linear_friction_matprop(isParamValid("linear_coef_name")),
    _use_quadratic_friction_matprop(isParamValid("quadratic_coef_name")),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _use_Forchheimer_friction_model(isParamValid("Forchheimer_name")),
    _momentum(getADMaterialProperty<RealVectorValue>(NS::momentum))
{
  if (!_use_linear_friction_matprop && !_use_quadratic_friction_matprop &&
      !_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");
}

ADReal
PNSFVMomentumFriction::computeQpResidual()
{
  ADReal friction_term = 0;
  if (_use_linear_friction_matprop)
    friction_term += (*_linear_friction_matprop)[_qp] * _momentum[_qp](_component);
  if (_use_quadratic_friction_matprop)
    friction_term += (*_quadratic_friction_matprop)[_qp] * _momentum[_qp](_component) *
                     std::abs(_momentum[_qp](_component));
  if (_use_Darcy_friction_model)
    friction_term += (*_cL)[_qp](_component) * _momentum[_qp](_component);
  if (_use_Forchheimer_friction_model)
    friction_term += (*_cQ)[_qp](_component) * _momentum[_qp](_component);

  return friction_term;
}
