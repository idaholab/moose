//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedNormalNoise.h"

registerMooseObject("PhaseFieldApp", ConservedNormalNoise);

template <>
InputParameters
validParams<ConservedNormalNoise>()
{
  auto params = validParams<ConservedNoiseBase>();
  params.addClassDescription("Gaussian normal distributed random number noise provider for the "
                             "ConservedLangevinNoise kernel.");
  return params;
}
