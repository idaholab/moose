//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PartialStatefulMaterial.h"

registerMooseObject("MooseTestApp", PartialStatefulMaterial);

InputParameters
PartialStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("initial_diffusivity", 1.0, "Initial value for diffusivity");
  params.addParam<Real>("initial_conductivity", 2.0, "Initial value for conductivity");
  return params;
}

PartialStatefulMaterial::PartialStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _initial_diffusivity(getParam<Real>("initial_diffusivity")),
    _initial_conductivity(getParam<Real>("initial_conductivity")),
    _diffusivity(declareProperty<Real>("diffusivity")),
    _diffusivity_old(getMaterialPropertyOld<Real>("diffusivity")),
    _conductivity(declareProperty<Real>("conductivity")),
    _conductivity_old(getMaterialPropertyOld<Real>("conductivity"))
{
}

void
PartialStatefulMaterial::initQpStatefulProperties()
{
  _diffusivity[_qp] = _initial_diffusivity;
  _conductivity[_qp] = _initial_conductivity;
}

void
PartialStatefulMaterial::computeQpProperties()
{
  // At the first timestep, verify the partial import behaved correctly:
  // - diffusivity_old must have been overwritten by the importer (not equal to initial_diffusivity)
  // - conductivity_old must still equal initial_conductivity, proving that
  //   initStatefulProperties() ran correctly for the non-imported property
  if (_t_step == 1)
  {
    if (MooseUtils::absoluteFuzzyEqual(_diffusivity_old[_qp], _initial_diffusivity))
      mooseError("diffusivity_old at qp ",
                 _qp,
                 " equals initial_diffusivity (",
                 _initial_diffusivity,
                 ") — the import did not overwrite it as expected.");
    if (!MooseUtils::absoluteFuzzyEqual(_conductivity_old[_qp], _initial_conductivity))
      mooseError("conductivity_old at qp ",
                 _qp,
                 " equals ",
                 _conductivity_old[_qp],
                 " but expected initial_conductivity (",
                 _initial_conductivity,
                 ") — initStatefulProperties() was not called for the non-imported property.");
  }

  _diffusivity[_qp] = _diffusivity_old[_qp] + _q_point[_qp](0) + _q_point[_qp](1);
  _conductivity[_qp] = _conductivity_old[_qp] + 0.1;
}
