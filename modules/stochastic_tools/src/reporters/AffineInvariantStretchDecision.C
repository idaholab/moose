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
  InputParameters params = ParallelMarkovChainMonteCarloDecision::validParams();
  params.addClassDescription("Perform decision making for Affine Invariant stretch MCMC.");
  return params;
}

AffineInvariantStretchDecision::AffineInvariantStretchDecision(const InputParameters & parameters)
  : ParallelMarkovChainMonteCarloDecision(parameters),
    _aiss(dynamic_cast<const AffineInvariantStretchSampler *>(&_sampler))
{
}

void
AffineInvariantStretchDecision::computeTransitionVector(std::vector<Real> & tv,
                                                        DenseMatrix<Real> & inputs_matrix)
{
  Real quant1;
  std::vector<Real> out1(_num_confg);
  std::vector<Real> out2(_num_confg);
  std::vector<Real> z = _aiss->getAffineStepSize();
  for (unsigned int i = 0; i < tv.size(); ++i)
  {
    quant1 = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      quant1 += (std::log(_priors[j]->pdf(inputs_matrix(i, j))) -
                 std::log(_priors[j]->pdf(_data_prev(i, j))));
    for (unsigned int j = 0; j < _num_confg; ++j)
    {
      out1[j] = _outputs_required[j * _pmcmc->getNumParallelProposals() + i];
      out2[j] = _outputs_prev[j * _pmcmc->getNumParallelProposals() + i];
    }
    for (unsigned int j = 0; j < _likelihoods.size(); ++j)
      quant1 += (_likelihoods[j]->function(out1) - _likelihoods[j]->function(out2));
    quant1 = std::exp(std::min((_priors.size() - 1) * std::log(z[i]) + quant1, 0.0));
    tv[i] = quant1;
  }
}
