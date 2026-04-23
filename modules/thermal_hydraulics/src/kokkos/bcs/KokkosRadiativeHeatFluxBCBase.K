//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosRadiativeHeatFluxBCBase.h"

InputParameters
KokkosRadiativeHeatFluxBCBase::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "The Stefan-Boltzmann constant.");
  params.addParam<Real>("Tinfinity", 0, "Temperature of the body in radiative heat transfer.");
  params.addClassDescription("Boundary condition for radiative heat flux where temperature and the"
                             "temperature of a body in radiative heat transfer are specified.");
  return params;
}

KokkosRadiativeHeatFluxBCBase::KokkosRadiativeHeatFluxBCBase(const InputParameters & parameters)
  : IntegratedBCValue(parameters),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _tinf(getParam<Real>("Tinfinity"))
{
}
