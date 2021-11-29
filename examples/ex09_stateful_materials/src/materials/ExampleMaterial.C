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
  params.addParam<Real>("initial_diffusivity", 1.0, "The Initial Diffusivity");
  return params;
}

ExampleMaterial::ExampleMaterial(const InputParameters & parameters)
  : Material(parameters),

    // Get a parameter value for the diffusivity
    _initial_diffusivity(getParam<Real>("initial_diffusivity")),

    // Declare that this material is going to have a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Retrieve/use an old value of diffusivity.
    // Note: this is _expensive_ as we have to store values for each
    // qp throughout the mesh. Only do this if you REALLY need it!
    _diffusivity_old(getMaterialPropertyOld<Real>("diffusivity"))
{
}

void
ExampleMaterial::initQpStatefulProperties()
{
  // init the diffusivity property (this will become
  // _diffusivity_old in the first call of computeProperties)
  _diffusivity[_qp] = _initial_diffusivity;
}

void
ExampleMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _diffusivity_old[_qp] * 2;
}
