//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Standardizer.h"

namespace StochasticTools
{

void
Standardizer::set(const Real & n)
{
  _mean.clear();
  _stdev.clear();
  for (unsigned int ii = 0; ii < n; ++ii)
  {
    _mean.push_back(0);
    _stdev.push_back(1);
  }
}

void
Standardizer::set(const Real & mean, const Real & stdev)
{
  _mean.clear();
  _stdev.clear();
  _mean.push_back(mean);
  _stdev.push_back(stdev);
}

void
Standardizer::set(const Real & mean, const Real & stdev, const Real & n)
{
  _mean.clear();
  _stdev.clear();
  for (unsigned int ii = 0; ii < n; ++ii)
  {
    _mean.push_back(mean);
    _stdev.push_back(stdev);
  }
}

void
Standardizer::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
{
  mooseAssert(mean.size() == stdev.size(),
              "Provided mean and standard deviation vectors are of differing size.");
  _mean = mean;
  _stdev = stdev;
}

void
Standardizer::computeSet(const RealEigenMatrix & input)
{
  _mean.clear();
  _stdev.clear();
  unsigned int num_samples = input.rows();
  unsigned int n = input.cols();
  // comptue mean
  RealEigenVector mean = input.colwise().mean();
  // Compute standard deviation
  RealEigenVector stdev =
      ((input.rowwise() - mean.transpose()).colwise().squaredNorm() / num_samples)
          .transpose()
          .array()
          .sqrt();
  // Store in std:vector format
  _mean.resize(n);
  _stdev.resize(n);
  RealEigenVector::Map(&_mean[0], n) = mean;
  RealEigenVector::Map(&_stdev[0], n) = stdev;
}

void
Standardizer::getStandardized(RealEigenMatrix & input) const
{
  Eigen::Map<const RealEigenVector> mean(_mean.data(), _mean.size());
  Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  input = (input.rowwise() - mean.transpose()).array().rowwise() / stdev.transpose().array();
}

void
Standardizer::getDestandardized(RealEigenMatrix & input) const
{
  Eigen::Map<const RealEigenVector> mean(_mean.data(), _mean.size());
  Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  input =
      (input.array().rowwise() * stdev.transpose().array()).rowwise() + mean.transpose().array();
}

void
Standardizer::getDescaled(RealEigenMatrix & input) const
{
  Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  input = input.array().rowwise() * stdev.transpose().array();
}

void
Standardizer::getScaled(RealEigenMatrix & input) const
{
  Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  input = input.array().rowwise() / stdev.transpose().array();
}

} // StochasticTools namespace
