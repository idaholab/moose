//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleMaterial.h"

registerMooseObject("ExampleApp", ExampleMaterial);

InputParameters
ExampleMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("diffusivity", 1.0, "The Diffusivity");
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleMaterial::ExampleMaterial(const InputParameters & parameters)
  : Material(parameters),

    // Get a parameter value for the diffusivity
    _input_diffusivity(getParam<Real>("diffusivity")),

    // Get a parameter value for the time_coefficient
    _input_time_coefficient(getParam<Real>("time_coefficient")),

    // Declare that this material is going to have a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Declare that this material is going to have a Real
    // valued property named "time_coefficient" that Kernels can use.
    _time_coefficient(declareProperty<Real>("time_coefficient"))
{
}

void
ExampleMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _input_diffusivity;
  _time_coefficient[_qp] = _input_time_coefficient;
}
