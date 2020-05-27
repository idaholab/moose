//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatialStatefulMaterial.h"

registerMooseObject("MooseTestApp", SpatialStatefulMaterial);

InputParameters
SpatialStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("initial_diffusivity", 0.5, "The Initial Diffusivity");
  return params;
}

SpatialStatefulMaterial::SpatialStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),

    // Get a parameter value for the diffusivity
    _initial_diffusivity(getParam<Real>("initial_diffusivity")),

    // Declare that this material is going to have a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Retrieve/use an old value of diffusivity.
    // Note: this is _expensive_ - only do this if you REALLY need it!
    _diffusivity_old(getMaterialPropertyOld<Real>("diffusivity"))
{
}

void
SpatialStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = _initial_diffusivity;
}

void
SpatialStatefulMaterial::computeQpProperties()
{
#ifndef NDEBUG
  if (_t_step == 1 && !MooseUtils::absoluteFuzzyEqual(_diffusivity_old[_qp], _initial_diffusivity))
    mooseError("stateful properties have not been properly intialized!");
#endif
  _diffusivity[_qp] = _diffusivity_old[_qp] + _q_point[_qp](0) + _q_point[_qp](1);
}
