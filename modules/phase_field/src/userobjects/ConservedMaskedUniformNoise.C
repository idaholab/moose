//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedMaskedUniformNoise.h"

registerMooseObject("PhaseFieldApp", ConservedMaskedUniformNoise);

InputParameters
ConservedMaskedUniformNoise::validParams()
{
  auto params = ConservedMaskedNoiseBase::validParams();
  params.addClassDescription("Uniformly distributed random number noise provider with an applied "
                             "spatially dependent material property mask for the "
                             "ConservedLangevinNoise kernel.");
  return params;
}
