//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef LIBTORCH_ENABLED

#include "Standardizer.h"

namespace StochasticTools
{

void
Standardizer::set(const Real & n)
{
  _mean = torch::zeros({long(n), 1}, at::kDouble);
  _stdev = torch::ones({long(n), 1}, at::kDouble);
}

void
Standardizer::set(const Real & mean, const Real & stdev)
{
  std::vector<Real> mean_vec;
  std::vector<Real> stdev_vec;
  mean_vec.push_back(mean);
  stdev_vec.push_back(stdev);

  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = torch::from_blob(mean_vec.data(), {1, 1}, options).to(at::kDouble);
  _stdev = torch::from_blob(stdev_vec.data(), {1, 1}, options).to(at::kDouble);
}

void
Standardizer::set(const Real & mean, const Real & stdev, const Real & n)
{
  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = mean * torch::ones({long(n), 1}, options).to(at::kDouble).clone();
  _stdev = stdev * torch::ones({long(n), 1}, options).to(at::kDouble).clone();
}

void
Standardizer::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
{
  mooseAssert(mean.size() == stdev.size(),
              "Provided mean and standard deviation vectors are of differing size.");
  auto mean_copy = mean;
  auto stdev_copy = stdev;
  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean =
      torch::from_blob(mean_copy.data(), {long(mean.size()), 1}, options).to(at::kDouble).clone();
  _stdev =
      torch::from_blob(stdev_copy.data(), {long(stdev.size()), 1}, options).to(at::kDouble).clone();
}

void
Standardizer::computeSet(const torch::Tensor & input)
{
  // comptue mean and standard deviation
  auto mean = torch::mean(input, 0, false);
  auto stdev = torch::std(input, 0, 0, false);
  mean = torch::resize(mean, {mean.sizes()[0], 1});
  stdev = torch::resize(stdev, {stdev.sizes()[0], 1});
  _mean = mean;
  _stdev = stdev;
}

void
Standardizer::getStandardized(torch::Tensor & input) const
{
  torch::Tensor mean = torch::transpose(_mean, 0, 1);
  torch::Tensor stdev = torch::transpose(_stdev, 0, 1);
  //  Standardize input tensor
  input = input - mean;
  input = input / stdev;
}

void
Standardizer::getDestandardized(torch::Tensor & input) const
{
  torch::Tensor mean = torch::transpose(_mean, 0, 1);
  torch::Tensor stdev = torch::transpose(_stdev, 0, 1);
  input = ((input * stdev) + mean);
}

void
Standardizer::getDescaled(torch::Tensor & input) const
{
  torch::Tensor stdev = torch::transpose(_stdev, 0, 1);
  input = (input * stdev);
}

/// Helper for dataStore
void
Standardizer::storeHelper(std::ostream & stream, void * context) const
{
  Real * temp_mean = _mean.data_ptr<Real>();
  Real * temp_stdev = _stdev.data_ptr<Real>();
  unsigned int n = _mean.size(0);
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, temp_mean[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, temp_stdev[ii], context);
}

} // StochasticTools namespace

template <>
void
dataStore(std::ostream & stream, StochasticTools::Standardizer & standardizer, void * context)
{
  standardizer.storeHelper(stream, context);
}

template <>
void
dataLoad(std::istream & stream, StochasticTools::Standardizer & standardizer, void * context)
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

#endif
