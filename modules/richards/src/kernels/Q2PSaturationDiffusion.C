//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Q2PSaturationDiffusion.h"

registerMooseObject("RichardsApp", Q2PSaturationDiffusion);

InputParameters
Q2PSaturationDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredParam<UserObjectName>(
      "fluid_relperm",
      "A RichardsRelPerm UserObject that defines the fluid relative permeability "
      "as a function of water saturation (eg RichardsRelPermPower)");
  params.addRequiredCoupledVar("porepressure_variable",
                               "The variable representing the porepressure");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addRequiredParam<Real>("diffusivity", "Diffusivity as a function of S");
  params.addClassDescription("Diffusion part of the Flux according to Darcy-Richards flow.  The "
                             "Variable of this Kernel must be the saturation.");
  return params;
}

Q2PSaturationDiffusion::Q2PSaturationDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _pp(coupledValue("porepressure_variable")),
    _pp_var_num(coupled("porepressure_variable")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _diffusivity(getParam<Real>("diffusivity"))
{
}

Real
Q2PSaturationDiffusion::computeQpResidual()
{
  Real coef = _diffusivity * _relperm.relperm(_u[_qp]) * _density.density(_pp[_qp]) / _viscosity;
  return coef * _grad_test[_i][_qp] * (_permeability[_qp] * _grad_u[_qp]);
}

Real
Q2PSaturationDiffusion::computeQpJacobian()
{
  Real coef = _diffusivity * _relperm.relperm(_u[_qp]) * _density.density(_pp[_qp]) / _viscosity;
  Real coefp = _diffusivity * _relperm.drelperm(_u[_qp]) * _density.density(_pp[_qp]) / _viscosity;
  return coefp * _phi[_j][_qp] * _grad_test[_i][_qp] * (_permeability[_qp] * _grad_u[_qp]) +
         coef * _grad_test[_i][_qp] * (_permeability[_qp] * _grad_phi[_j][_qp]);
}

Real
Q2PSaturationDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _pp_var_num)
    return 0.0;
  Real coefp = _diffusivity * _relperm.relperm(_u[_qp]) * _density.ddensity(_pp[_qp]) / _viscosity;
  return coefp * _phi[_j][_qp] * (_grad_test[_i][_qp] * (_permeability[_qp] * _grad_u[_qp]));
}
