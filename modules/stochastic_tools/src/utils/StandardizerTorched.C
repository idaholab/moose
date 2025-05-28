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
  std::vector<Real> mean;
  std::vector<Real> stdev;
  //_mean.clear();
  //_stdev.clear();
  for (unsigned int ii = 0; ii < n; ++ii)
  {
    mean.push_back(0);
    stdev.push_back(1);
  }
  auto options = torch::TensorOptions().dtype(at::kDouble);
  unsigned int num_samples = n;
  unsigned int num_inputs = 1;
  _mean = torch::from_blob(mean.data(), {num_samples, num_inputs}, options).to(at::kDouble);
  //_mean = torch::from_blob(mean_vector);
  _stdev = torch::from_blob(stdev.data(), {num_samples, num_inputs}, options).to(at::kDouble);
}

void
StandardizerTorched::set(const Real & mean, const Real & stdev)
{
  //_mean.clear();
  //_stdev.clear();
  //_mean.push_back(mean);
  //_stdev.push_back(stdev);

  std::vector<Real> mean_vec;
  std::vector<Real> stdev_vec;
  mean_vec.push_back(mean);
  stdev_vec.push_back(stdev);

  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = torch::from_blob(mean_vec.data(), {1, 1}, options).to(at::kDouble);
  _stdev = torch::from_blob(stdev_vec.data(), {1, 1}, options).to(at::kDouble);
}

void
StandardizerTorched::set(const Real & mean, const Real & stdev, const Real & n)
{
  std::vector<Real> mean_vec;
  std::vector<Real> stdev_vec;
  for (unsigned int ii = 0; ii < n; ++ii)
  {
    mean_vec.push_back(mean);
    stdev_vec.push_back(stdev);
  }
  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = torch::from_blob(mean_vec.data(), {long(n), 1}, options).to(at::kDouble).clone();
  _stdev = torch::from_blob(stdev_vec.data(), {long(n), 1}, options).to(at::kDouble).clone();
}

void
StandardizerTorched::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
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

  /*
  auto options = torch::TensorOptions().dtype(torch::kFloat32);
  _mean = torch::empty({mean.size()}, options);
  _stdev = torch::empty({stdev.size()}, options);
  std::copy_n(mean, mean.size(), _mean.contiguous().data_ptr<Real>());
  std::copy_n(stdev, stdev.size(), _stdev.contiguous().data_ptr<Real>());
  */
}

void
StandardizerTorched::computeSet(const torch::Tensor & input)
{
  // comptue mean and standard deviation
  auto mean = torch::mean(input, 0, false);
  auto stdev = torch::std(input, 0, 0, true);
  _mean = mean;
  _stdev = stdev;
}

void
StandardizerTorched::getStandardized(torch::Tensor & input) const
{
  // Standardize input tensor
  input = (input - torch::mean(input, 0, false));
  input = input / torch::std(input, 0, 0, true);
}

void
StandardizerTorched::getDestandardized(torch::Tensor & input) const
{
  input = (input * torch::std(input, 0, 0, true) + torch::mean(input, 0, false));
}

void
StandardizerTorched::getDescaled(torch::Tensor & input) const
{
  // Eigen::Map<const RealEigenVector> stdev(_stdev.data(), _stdev.size());
  // input = input.array().rowwise() * stdev.transpose().array();
  input = input * torch::std(input, 0, 0, true);
}

/// Helper for dataStore
void
StandardizerTorched::storeHelper(std::ostream & stream, void * context) const
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
