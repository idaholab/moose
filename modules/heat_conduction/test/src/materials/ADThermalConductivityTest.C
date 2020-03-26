//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADThermalConductivityTest.h"

registerMooseObject("HeatConductionTestApp", ADThermalConductivityTest);

template <ComputeStage compute_stage>
InputParameters
ADThermalConductivityTest<compute_stage>::validParams()
{
  auto params = ADMaterial<compute_stage>::validParams();
  params.addRequiredCoupledVar("temperature", "Coupled temperature");
  params.addRequiredCoupledVar(
      "c", "Coupled variable used to help verify automatic differentiation capability");
  return params;
}

template <ComputeStage compute_stage>
ADThermalConductivityTest<compute_stage>::ADThermalConductivityTest(
    const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _diffusivity(declareADProperty<Real>("thermal_conductivity")),
    _temperature(adCoupledValue("temperature")),
    _c(adCoupledValue("c"))
{
}

template <ComputeStage compute_stage>
void
ADThermalConductivityTest<compute_stage>::computeQpProperties()
{
  _diffusivity[_qp] = _temperature[_qp] * _c[_qp];
}
