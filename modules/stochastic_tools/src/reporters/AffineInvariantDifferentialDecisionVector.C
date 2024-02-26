//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AffineInvariantDifferentialDecisionVector.h"

registerMooseObject("StochasticToolsApp", AffineInvariantDifferentialDecisionVector);

InputParameters
AffineInvariantDifferentialDecisionVector::validParams()
{
  InputParameters params = PMCMCDecisionVector::validParams();
  params.addClassDescription("Perform decision making for Affine Invariant differential MCMC.");
  return params;
}

AffineInvariantDifferentialDecisionVector::AffineInvariantDifferentialDecisionVector(
    const InputParameters & parameters)
  : PMCMCDecisionVector(parameters), _aides(dynamic_cast<const AffineInvariantDES *>(&_sampler))
{
  // Check whether the selected sampler is a differential evolution sampler or not
  if (!_aides)
    paramError("sampler", "The selected sampler is not of type AffineInvariantDES.");
}

void
AffineInvariantDifferentialDecisionVector::computeTransitionVector(std::vector<Real> & tv,
                                                             const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min(evidence[i], 0.0));
}
