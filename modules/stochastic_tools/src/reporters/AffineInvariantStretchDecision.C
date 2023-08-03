//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantStretchDecision.h"

registerMooseObject("StochasticToolsApp", AffineInvariantStretchDecision);

InputParameters
AffineInvariantStretchDecision::validParams()
{
  InputParameters params = PMCMCDecision::validParams();
  params.addClassDescription("Perform decision making for Affine Invariant stretch MCMC.");
  return params;
}

AffineInvariantStretchDecision::AffineInvariantStretchDecision(const InputParameters & parameters)
  : PMCMCDecision(parameters),
    _aiss(dynamic_cast<const AffineInvariantStretchSampler *>(&_sampler)),
    _step_size(_aiss->getAffineStepSize())
{
  // Check whether the selected sampler is a stretch sampler or not
  if (!_aiss)
    paramError("sampler", "The selected sampler is not of type AffineInvariantStretchSampler.");
}

void
AffineInvariantStretchDecision::computeTransitionVector(std::vector<Real> & tv,
                                                        const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min((_priors.size() - 1) * std::log(_step_size[i]) + evidence[i], 0.0));
}
