//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "SoundspeedMat.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", SoundspeedMat);

InputParameters
SoundspeedMat::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addClassDescription("Computes the speed of sound");
  return params;
}

SoundspeedMat::SoundspeedMat(const InputParameters & parameters)
  : Material(parameters),
    _sound_speed(declareADProperty<Real>(NS::sound_speed)),
    _pressure(getADMaterialProperty<Real>(NS::pressure)),
    _temperature(getADMaterialProperty<Real>(NS::T_fluid)),
    _fluid(getUserObject<SinglePhaseFluidProperties>(NS::fluid))
{
}

void
SoundspeedMat::computeQpProperties()
{
  _sound_speed[_qp] = _fluid.c_from_p_T(_pressure[_qp], _temperature[_qp]);
}
