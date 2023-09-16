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
  unsigned int siz = latent.size() / 2;
  RealEigenMatrix B1(siz, 1);
  RealEigenVector B2(siz);
  unsigned int count = 0;
  for (unsigned int ii = 0; ii < siz; ++ii)
  {
    B1(ii, 0) = latent[count];
    ++count;
  }
  for (unsigned int ii = 0; ii < siz; ++ii)
  {
    B2(ii) = latent[count];
    ++count;
  }
  RealEigenMatrix tmp = B2.asDiagonal();
  B = B1 * B1.transpose() + tmp;
}

void
IntrinsicCoregionalizationModel::computeBGrad(RealEigenMatrix & BGrad,
                                              const std::vector<Real> & latent,
                                              const unsigned int & index) const
{
  unsigned int siz = latent.size() / 2;
  RealEigenVector B1(siz);
  BGrad.Zero(siz, siz);
  if (index < siz)
  {
    for (unsigned int ii = 0; ii < siz; ++ii)
      B1(ii) = latent[ii];
    BGrad.col(index) = B1;
    BGrad = BGrad + BGrad.transpose();
  }
  else
    BGrad(index, index) = 1.0;
}

unsigned int
IntrinsicCoregionalizationModel::setupNumLatent(const unsigned int & num_outputs) const
{
  return num_outputs * 2;
}
