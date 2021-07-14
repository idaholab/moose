//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVEnergyTimeDerivative);
registerMooseObjectRenamed("NavierStokesApp",
                           PINSFVEnergyTimeDerivative,
                           "01/01/2022 00:00",
                           PNSFVEnergyTimeDerivative);

InputParameters
PNSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the Navier-Stokes energy equation: "
      "for fluids: eps * rho * cp * dT/dt, for solids: (1 - eps) * rho * cp * dT/dt");
  params.addRequiredParam<MaterialPropertyName>("rho", "The value for the density");
  params.addRequiredParam<MaterialPropertyName>("cp_name",
                                                "The name of the specific heat capacity");
  params.addCoupledVar("porosity", "Porosity variable");
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

PNSFVEnergyTimeDerivative::PNSFVEnergyTimeDerivative(const InputParameters & params)
  : FVTimeKernel(params),
    _rho(getADMaterialProperty<Real>("rho")),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _eps(isCoupled("porosity") ? coupledValue("porosity")
                               : getMaterialProperty<Real>("porosity_prop_name").get()),
    _is_solid(getParam<bool>("is_solid")),
    _scaling(getParam<Real>("scaling")),
    _zero_scaling(_scaling < 1e-8)
{
}

ADReal
PNSFVEnergyTimeDerivative::computeQpResidual()
{
  if (_zero_scaling)
    return 0.0;
  else
    // Note: This is neglecting time derivative of both _cp and _rho
    return _scaling * (_is_solid ? 1 - _eps[_qp] : _eps[_qp]) * _rho[_qp] * _cp[_qp] *
           FVTimeKernel::computeQpResidual();
}
