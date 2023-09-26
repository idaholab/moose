//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatFluxTest.h"

registerMooseObject("HeatConductionTestApp", ADConvectiveHeatFluxTest);

InputParameters
ADConvectiveHeatFluxTest::validParams()
{
  auto params = Material::validParams();
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  return params;
}

ADConvectiveHeatFluxTest::ADConvectiveHeatFluxTest(const InputParameters & parameters)
  : Material(parameters),
    _temperature(adCoupledValue("temperature")),
    _t_inf(declareADProperty<Real>("T_inf")),
    _htc(declareADProperty<Real>("htc"))
{
}

void
ADConvectiveHeatFluxTest::computeQpProperties()
{
  _t_inf[_qp] = _temperature[_qp] + 1;
  _htc[_qp] = _temperature[_qp] / 100 + 1;
}
