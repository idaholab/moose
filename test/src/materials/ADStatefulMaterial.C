//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStatefulMaterial.h"

registerMooseObject("MooseTestApp", ADStatefulMaterial);

InputParameters
ADStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("initial_diffusivity", 0.5, "The Initial Diffusivity");
  params.addRequiredCoupledVar("u", "The coupled variable");
  return params;
}

ADStatefulMaterial::ADStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),

    // Get a parameter value for the diffusivity
    _initial_diffusivity(getParam<Real>("initial_diffusivity")),

    // Declare that this material is going to have a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareADProperty<Real>("diffusivity")),

    // Retrieve/use an old value of diffusivity.
    // Note: this is _expensive_ - only do this if you REALLY need it!
    _diffusivity_old(getMaterialPropertyOld<Real>("diffusivity")),
    _u(adCoupledValue("u"))
{
}

void
ADStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = _initial_diffusivity;
}

void
ADStatefulMaterial::computeQpProperties()
{
  _diffusivity[_qp] = _diffusivity_old[_qp] * 2.0 * _u[_qp];
}
