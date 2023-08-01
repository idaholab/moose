//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AdaptiveImportanceSampler.h"
#include "ReporterInterface.h"

/**
 * A class used to perform Adaptive Importance Sampling using a Markov Chain Monte Carlo algorithm
 * and Gaussian Process active learning
 */
class AISActiveLearning : public AdaptiveImportanceSampler
{
public:
  static InputParameters validParams();

  AISActiveLearning(const InputParameters & parameters);
};
