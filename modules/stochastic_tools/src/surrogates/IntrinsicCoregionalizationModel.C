//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntrinsicCoregionalizationModel.h"
#include <cmath>

registerMooseObject("StochasticToolsApp", IntrinsicCoregionalizationModel);

InputParameters
IntrinsicCoregionalizationModel::validParams()
{
  InputParameters params = OutputCovarianceBase::validParams();
  params.addClassDescription("The ICM covariance.");
  return params;
}

IntrinsicCoregionalizationModel::IntrinsicCoregionalizationModel(const InputParameters & parameters)
  : OutputCovarianceBase(parameters)
{
}

void
IntrinsicCoregionalizationModel::computeBCovarianceMatrix(RealEigenMatrix & B,
                                                          const std::vector<Real> & latent) const
{
}

void
IntrinsicCoregionalizationModel::computeFullCovarianceMatrix(RealEigenMatrix & kappa,
                                                             const RealEigenMatrix & B,
                                                             const RealEigenMatrix & K) const
{
}

void
IntrinsicCoregionalizationModel::computeBGrad(RealEigenMatrix & BGrad,
                                              const std::vector<Real> & latent,
                                              const unsigned int & index) const
{
}

unsigned int
IntrinsicCoregionalizationModel::setupNumLatent(const unsigned int & num_outputs) const
{
  return num_outputs * 2;
}
