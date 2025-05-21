//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Standardizer.h"

namespace StochasticToolsTorched
{

void
StandardizerTorched::set(const Real & n)
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
StandardizerTorched::set(const Real & mean, const Real & stdev)
{
  _mean.clear();
  _stdev.clear();
  _mean.push_back(mean);
  _stdev.push_back(stdev);
}

void
StandardizerTorched::set(const Real & mean, const Real & stdev, const Real & n)
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
StandardizerTorched::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
{
  mooseAssert(mean.size() == stdev.size(),
              "Provided mean and standard deviation vectors are of differing size.");
  _mean = mean;
  _stdev = stdev;
}

void
StandardizerTorched::computeSet(const torch::Tensor & input)
{
  _mean.clear();
  _stdev.clear();
  unsigned int num_samples = input.sizes()[0];
  unsigned int n = input.sizes()[1];

  // TODO: mean and stdev can be calculated using libtorch std and std_mean if torch tensor is
  // provided

  // comptue mean
  // RealEigenVector mean = input.colwise().mean();
  auto mean = torch::mean(input, 1, true);
  // Compute standard deviation
  /*
  RealEigenVector stdev =
      ((input.rowwise() - mean.transpose()).colwise().squaredNorm() / num_samples)
          .transpose()
          .array()
          .sqrt();
  */
  // Store in std:vector format
  auto stdev = torch::std(input, 1, true);
  _mean.resize(n);
  _stdev.resize(n);
  // RealEigenVector::Map(&_mean[0], n) = mean;
  // RealEigenVector::Map(&_stdev[0], n) = stdev;
}

void
StandardizerTorched::getStandardized(torch::Tensor & input) const
{
  // Eigen::Map<const RealEigenVector> mean(_mean.data(), _mean.size());
  // Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  // input = (input.rowwise() - mean.transpose()).array().rowwise() / stdev.transpose().array();
  input = (input - torch::mean(input, 1, true));
  input = input / torch::std(input);
}

void
StandardizerTorched::getDestandardized(torch::Tensor & input) const
{
  // Eigen::Map<const RealEigenVector> mean(_mean.data(), _mean.size());
  // Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  // input =
  //     (input.array().rowwise() * stdev.transpose().array()).rowwise() + mean.transpose().array();
  input = (input * torch::std(input, 1, true) + torch::mean(input, 1, true));
}

void
StandardizerTorched::getDescaled(torch::Tensor & input) const
{
  // Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  // input = input.array().rowwise() * stdev.transpose().array();
  input = input * torch::std(input, 1, true);
}

/// Helper for dataStore
void
StandardizerTorched::storeHelper(std::ostream & stream, void * context) const
{
  unsigned int n = _mean.size();
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, _mean[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, _stdev[ii], context);
}

} // StochasticTools namespace

template <>
void
dataStore(std::ostream & stream,
          StochasticToolsTorched::StandardizerTorched & standardizer,
          void * context)
{
  standardizer.storeHelper(stream, context);
}

template <>
void
dataLoad(std::istream & stream,
         StochasticToolsTorched::StandardizerTorched & standardizer,
         void * context)
{
  unsigned int n;
  dataLoad(stream, n, context);
  std::vector<Real> mean(n);
  std::vector<Real> stdev(n);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataLoad(stream, mean[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataLoad(stream, stdev[ii], context);
  standardizer.set(mean, stdev);
}
