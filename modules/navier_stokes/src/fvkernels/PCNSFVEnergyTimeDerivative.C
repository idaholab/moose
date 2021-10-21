//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVEnergyTimeDerivative);

InputParameters
PCNSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the Navier-Stokes energy equation: "
      "for fluids: eps * d(rho * cp * T)/dt, for solids: (1 - eps) * d(rho * cp * T)/dt. "
      "Material property derivatives are ignored if not provided.");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "Density material property");
  params.addRequiredParam<MaterialPropertyName>(NS::time_deriv(NS::density),
                                                "Density time derivative material property");
  params.addRequiredParam<MaterialPropertyName>(NS::cp, "Specific heat capacity material property");
  params.addRequiredParam<MaterialPropertyName>(
      NS::time_deriv(NS::cp), "Specific heat capacity time derivative material property");

  params.addCoupledVar(NS::porosity, "Porosity variable");
  params.addParam<MaterialPropertyName>("porosity_prop_name",
                                        "A name for a porosity material property.");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");
  params.addRangeCheckedParam<Real>("scaling",
                                    1,
                                    "scaling >= 0.0",
                                    "scaling factor to reduce the thermal mass during pseudo "
                                    "transients; this can accelerate convergence to steady state");
  return params;
}

PCNSFVEnergyTimeDerivative::PCNSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getADMaterialProperty<Real>("rho")),
    _rho_dot(&getADMaterialProperty<Real>(NS::time_deriv(NS::density))),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _cp_dot(&getADMaterialProperty<Real>(NS::time_deriv(NS::cp))),
    _eps(isCoupled("porosity") ? coupledValue("porosity")
                               : getMaterialProperty<Real>("porosity_prop_name").get()),
    _is_solid(getParam<bool>("is_solid")),
    _scaling(getParam<Real>("scaling")),
    _zero_scaling(_scaling < 1e-8)
{
}

ADReal
PCNSFVEnergyTimeDerivative::computeQpResidual()
{
  if (_zero_scaling)
    return 0.0;
  else
  {
    auto derivative = _rho[_qp] * _cp[_qp] * FVTimeKernel::computeQpResidual();
    derivative += (*_rho_dot)[_qp] * _cp[_qp] * _var.getElemValue(_current_elem);
    derivative += _rho[_qp] * (*_cp_dot)[_qp] * _var.getElemValue(_current_elem);

    return _scaling * (_is_solid ? 1 - _eps[_qp] : _eps[_qp]) * derivative;
  }
}
