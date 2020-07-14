//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFluidEnergySpecifiedTemperatureBC.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

namespace nms = NS;

// Full specialization of the validParams function for this object
registerADMooseObject("NavierStokesApp", CNSFVFluidEnergySpecifiedTemperatureBC);

InputParameters
CNSFVFluidEnergySpecifiedTemperatureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addParam<Real>(nms::porosity, 1, "porosity");
  params.addRequiredParam<PostprocessorName>("specified_temperature", "Specified temperature.");
  params.addRequiredParam<UserObjectName>(nms::fluid, "Fluid properties userobject");
  return params;
}

CNSFVFluidEnergySpecifiedTemperatureBC::CNSFVFluidEnergySpecifiedTemperatureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _eps(getParam<Real>(nms::porosity)),
    _temperature(this->getPostprocessorValue("specified_temperature")),
    _velocity(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(nms::fluid))
{
}

ADReal
CNSFVFluidEnergySpecifiedTemperatureBC::computeQpResidual()
{
  ADReal T = _temperature;
  ADReal rho = _fluid.rho_from_p_T(_pressure[_qp], T);
  ADReal H = _fluid.e_from_p_T(_pressure[_qp], T) + 0.5 * _velocity[_qp] * _velocity[_qp] + _pressure[_qp] / rho;
  return _normal * _velocity[_qp] * H * rho;
}
