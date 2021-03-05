//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVSolidEnergyTime.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVSolidEnergyTime);

InputParameters
PNSFVSolidEnergyTime::validParams()
{
  auto params = FVTimeKernel::validParams();
  params.addRangeCheckedParam<Real>("scaling",
                                    1,
                                    "scaling >= 0.0",
                                    "scaling factor to reduce the thermal mass during pseudo "
                                    "transients; this can accelerate convergence to steady state");
  params.addClassDescription(
      "Time derivative $(1-\\epsilon)\\rho_sC_{p,s}\\frac{\\partial T_s}{\\partial t}$ "
      "in the solid energy conservation equation.");
  return params;
}

PNSFVSolidEnergyTime::PNSFVSolidEnergyTime(const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _eps(getMaterialProperty<Real>(NS::porosity)),
    _scaling(getParam<Real>("scaling")),
    _zero_scaling(_scaling < NS_DEFAULT_VALUES::epsilon),
    _rho_s(_zero_scaling ? nullptr : &getADMaterialProperty<Real>(NS::rho_s)),
    _cp_s(_zero_scaling ? nullptr : &getADMaterialProperty<Real>(NS::cp_s))
{
}

ADReal
PNSFVSolidEnergyTime::computeQpResidual()
{
  if (_zero_scaling)
    return 0.0;
  else
    return _scaling * (1 - _eps[_qp]) * (*_rho_s)[_qp] * (*_cp_s)[_qp] *
           FVTimeKernel::computeQpResidual();
}
