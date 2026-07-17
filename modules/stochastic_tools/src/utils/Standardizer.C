//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

#include "Standardizer.h"

namespace StochasticTools
{

namespace
{

void
checkInputCompatibility(const torch::Tensor & input, const torch::Tensor & reference)
{
  if (input.dim() != 2)
    mooseError("Standardizer input must be a rank-2 tensor.");
  if (reference.dim() != 1)
    mooseError("Standardizer moments must be stored as feature vectors.");
  if (input.size(1) != reference.size(0))
    mooseError("Standardizer input dimension mismatch.");
}

torch::Tensor
toStandardizerOptions(const torch::Tensor & tensor, const torch::TensorOptions & options)
{
  auto result = tensor.to(options.device());
  if (result.scalar_type() != at::kDouble)
    result = result.to(at::kDouble);
  return result;
}

torch::Tensor
asFeatureVector(const torch::Tensor & feature_vector, const torch::Tensor & input)
{
  return toStandardizerOptions(feature_vector, input.options().dtype(at::kDouble));
}

} // namespace

void
Standardizer::set(const Real & n)
{
  _mean = torch::zeros({long(n)}, at::kDouble);
  _stdev = torch::ones({long(n)}, at::kDouble);
}

void
Standardizer::set(const Real & mean, const Real & stdev)
{
  _mean = torch::full({1}, mean, at::kDouble);
  _stdev = torch::full({1}, stdev, at::kDouble);
}

void
Standardizer::set(const Real & mean, const Real & stdev, const Real & n)
{
  auto options = torch::TensorOptions().dtype(at::kDouble);
  _mean = torch::full({long(n)}, mean, options);
  _stdev = torch::full({long(n)}, stdev, options);
}

void
Standardizer::set(const std::vector<Real> & mean, const std::vector<Real> & stdev)
{
  mooseAssert(mean.size() == stdev.size(),
              "Provided mean and standard deviation vectors are of differing size.");
  _mean = LibtorchUtils::vectorToTensorCopy(mean, {long(mean.size())});
  _stdev = LibtorchUtils::vectorToTensorCopy(stdev, {long(stdev.size())});
}

void
Standardizer::computeSet(const torch::Tensor & input)
{
  if (input.dim() != 2)
    mooseError("Standardizer input must be a rank-2 tensor.");
  // Compute mean and standard deviation
  _mean = torch::mean(input, 0, false);
  _stdev = torch::std(input, 0, 0, false);
}

void
Standardizer::getStandardized(torch::Tensor & input) const
{
  checkInputCompatibility(input, _mean);
  input.sub_(asFeatureVector(_mean, input)).div_(asFeatureVector(_stdev, input));
}

void
Standardizer::getDestandardized(torch::Tensor & input) const
{
  checkInputCompatibility(input, _mean);
  input.mul_(asFeatureVector(_stdev, input)).add_(asFeatureVector(_mean, input));
}

void
Standardizer::getDescaled(torch::Tensor & input) const
{
  checkInputCompatibility(input, _stdev);
  input.mul_(asFeatureVector(_stdev, input));
}

void
Standardizer::getScaled(torch::Tensor & input) const
{
  checkInputCompatibility(input, _stdev);
  input.div_(asFeatureVector(_stdev, input));
}

/// Helper for dataStore
void
Standardizer::storeHelper(std::ostream & stream, void * context) const
{
  const auto mean = LibtorchUtils::toCPUContiguous(_mean);
  const auto stdev = LibtorchUtils::toCPUContiguous(_stdev);
  auto mean_accessor = mean.accessor<Real, 1>();
  auto stdev_accessor = stdev.accessor<Real, 1>();
  unsigned int n = mean.size(0);
  dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, mean_accessor[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    dataStore(stream, stdev_accessor[ii], context);
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
