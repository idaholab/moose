//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVMomentumFriction.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVMomentumFriction);

InputParameters
PCNSFVMomentumFriction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Computes a friction force term on fluid in porous media in the "
                             "Navier Stokes i-th momentum equation.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<MaterialPropertyName>("Darcy_name",
                                        "Name of the Darcy coefficients material property.");
  params.addParam<MaterialPropertyName>("Forchheimer_name",
                                        "Name of the Forchheimer coefficients material property.");
  params.addCoupledVar(NS::porosity, "Porosity variable.");
  params.addRequiredParam<MaterialPropertyName>(
      "momentum_name",
      "Name of the superficial momentum material property for "
      "the Darcy and Forchheimer friction terms.");
  return params;
}

PCNSFVMomentumFriction::PCNSFVMomentumFriction(const InputParameters & params)
  : FVElementalKernel(params),
    _component(getParam<MooseEnum>("momentum_component")),
    _cL(isParamValid("Darcy_name") ? &getADMaterialProperty<RealVectorValue>("Darcy_name")
                                   : nullptr),
    _cQ(isParamValid("Forchheimer_name")
            ? &getADMaterialProperty<RealVectorValue>("Forchheimer_name")
            : nullptr),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _use_Forchheimer_friction_model(isParamValid("Forchheimer_name")),
    _eps(isCoupled(NS::porosity) ? coupledValue(NS::porosity)
                                 : getMaterialProperty<Real>(NS::porosity).get()),
    _momentum(getADMaterialProperty<Real>("momentum_name"))
{
  if (!_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");
}

ADReal
PCNSFVMomentumFriction::computeQpResidual()
{
  ADReal friction_term = 0;

  if (_use_Darcy_friction_model)
    friction_term += (*_cL)[_qp](_component) * _momentum[_qp] / _eps[_qp];
  if (_use_Forchheimer_friction_model)
    friction_term += (*_cQ)[_qp](_component) * _momentum[_qp] / _eps[_qp];

  return friction_term;
}
