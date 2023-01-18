//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonsafeMaterial.h"
#include "MooseApp.h"

registerMooseObject("MooseTestApp", NonsafeMaterial);

InputParameters
NonsafeMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<Real>("diffusivity", "The diffusivity value.");
  params.addParam<Real>(
      "threshold",
      0,
      "This value sets the upper limit for the validity of the material property value.");
  return params;
}

NonsafeMaterial::NonsafeMaterial(const InputParameters & parameters)
  : Material(parameters),
    _input_diffusivity(getParam<Real>("diffusivity")),
    _threshold(getParam<Real>("threshold")),
    _diffusivity(declareProperty<Real>("diffusivity"))
{
}

void
NonsafeMaterial::computeQpProperties()
{
  if (_input_diffusivity > _threshold)
  {
    flagInvalidSolution("Diffusivity check",
                        "The diffusivity is greater than the threshold value!");
    flagInvalidSolution("Second thing to check", "Extra invalid thing!");
  }
  _diffusivity[_qp] = _input_diffusivity;
}
