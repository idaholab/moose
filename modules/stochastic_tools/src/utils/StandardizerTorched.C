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
  _mean = torch::from_blob(mean_vec.data(), {n, 1}, options).to(at::kDouble);
  _stdev = torch::from_blob(stdev_vec.data(), {n, 1}, options).to(at::kDouble);
}

void
StandardizerTorched::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
{
  mooseAssert(mean.size() == stdev.size(),
              "Provided mean and standard deviation vectors are of differing size.");
  auto mean_copy = mean;
  auto stdev_copy = stdev;
  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = torch::from_blob(mean_copy.data(), {mean.size(), 1}, options).to(at::kDouble);
  _stdev = torch::from_blob(stdev_copy.data(), {stdev.size(), 1}, options).to(at::kDouble);
}

void
StandardizerTorched::computeSet(const torch::Tensor & input)
{
  //_mean.clear();
  //_stdev.clear();
  // unsigned int num_samples = input.sizes()[0];
  // unsigned int n = input.sizes()[1];

  // TODO: mean and stdev can be calculated using libtorch std and std_mean if torch tensor is
  // provided

  // comptue mean
  // RealEigenVector mean = input.colwise().mean();
  auto mean = torch::mean(input, 0, false);
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
  //_mean.resize(n);
  //_stdev.resize(n);
  _mean = mean;
  _stdev = stdev;

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
  Real * temp_mean = _mean.data<Real>();
  Real * temp_stdev = _stdev.data<Real>();
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
