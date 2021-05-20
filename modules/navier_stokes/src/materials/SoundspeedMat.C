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

registerADMooseObject("NavierStokesApp", SoundspeedMat);

namespace nms = NS;

defineADValidParams(SoundspeedMat,
                    ADMaterial,
                    params.addRequiredParam<UserObjectName>(nms::fluid, "fluid userobject");
                    params.addClassDescription("Computes the speed of sound"););

SoundspeedMat::SoundspeedMat(const InputParameters & parameters)
  : ADMaterial(parameters),
    _sound_speed(declareADProperty<Real>(nms::sound_speed)),
    _pressure(getADMaterialProperty<Real>(nms::pressure)),
    _temperature(getADMaterialProperty<Real>(nms::T_fluid)),
    _fluid(getUserObject<SinglePhaseFluidProperties>(nms::fluid))
{
}

void
SoundspeedMat::computeQpProperties()
{
  _sound_speed[_qp] = _fluid.c_from_p_T(_pressure[_qp], _temperature[_qp]);
}
