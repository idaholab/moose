//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AISActiveLearning.h"

registerMooseObject("StochasticToolsApp", AISActiveLearning);

InputParameters
AISActiveLearning::validParams()
{
  InputParameters params = AdaptiveImportanceSampler::validParams();
  params.addClassDescription("Adaptive Importance Sampler with Gaussian Process Active Learning.");
  return params;
}

AISActiveLearning::AISActiveLearning(const InputParameters & parameters)
  : AdaptiveImportanceSampler(parameters)
{
}
