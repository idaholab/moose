//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImportCheckStatefulMaterial.h"

registerMooseObject("MooseTestApp", ImportCheckStatefulMaterial);

InputParameters
ImportCheckStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("initial_diffusivity", 10.0, "Initial value for diffusivity");
  return params;
}

ImportCheckStatefulMaterial::ImportCheckStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _initial_diffusivity(getParam<Real>("initial_diffusivity")),
    _diffusivity(declareProperty<Real>("diffusivity")),
    _diffusivity_old(getMaterialPropertyOld<Real>("diffusivity"))
{
}

void
ImportCheckStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = _initial_diffusivity;
}

void
ImportCheckStatefulMaterial::computeQpProperties()
{
  if (_t_step == 1 && MooseUtils::absoluteFuzzyEqual(_diffusivity_old[_qp], _initial_diffusivity))
    mooseError("diffusivity_old at qp ",
               _qp,
               " equals initial_diffusivity (",
               _initial_diffusivity,
               ") - the import did not overwrite it as expected.");

  _diffusivity[_qp] = _diffusivity_old[_qp] + _q_point[_qp](0) + _q_point[_qp](1);
}
