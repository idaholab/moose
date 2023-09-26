//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatHeatSource.h"

registerMooseObject("HeatConductionApp", ADMatHeatSource);

InputParameters
ADMatHeatSource::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addParam<Real>("scalar", 1.0, "Scalar multiplied by the body force term");
  params.addParam<MaterialPropertyName>(
      "material_property", 1.0, "Material property describing the body force");
  params.addClassDescription("Force term in thermal transport to represent a heat source");
  return params;
}

ADMatHeatSource::ADMatHeatSource(const InputParameters & parameters)
  : ADKernel(parameters),
    _scalar(getParam<Real>("scalar")),
    _material_property(getADMaterialProperty<Real>("material_property"))
{
}

ADReal
ADMatHeatSource::computeQpResidual()
{
  return -_scalar * _material_property[_qp] * _test[_i][_qp];
}
