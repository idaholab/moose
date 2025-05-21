//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <vector>
#include "DataIO.h"

#include "LibtorchUtils.h"

namespace StochasticToolsTorched
{

/// Class for standardizing data (centering and scaling)

class StandardizerTorched
{
public:
  StandardizerTorched() = default;

  /// Methods for setting mean and standard deviation directly
  /// Sets mean=0, std=1 for n variables
  void set(const Real & n);
  /// Sets mean and std for a single variable
  void set(const Real & mean, const Real & stdev);
  /// Sets mean and std for a n variables variable
  void set(const Real & mean, const Real & stdev, const Real & n);
  /// Sets mean and std directly using provided vectors
  void set(const std::vector<Real> & mean, const std::vector<Real> & stdev);

  /// Get the mean vector
  const std::vector<Real> & getMean() const { return _mean; }
  /// Get the standard deviation vector
  const std::vector<Real> & getStdDev() const { return _stdev; }

  /// Methods for computing and setting mean and standard deviation
  void computeSet(const torch::Tensor & input);

  /// Helper for dataStore
  void storeHelper(std::ostream & stream, void * context) const;

  /// Returns the standardized (centered and scaled) of the provided input
  void getStandardized(torch::Tensor & input) const;

  /// De-standardizes (de-centered and de-scaled) the assumed standardized input
  void getDestandardized(torch::Tensor & input) const;

  /// De-scales the assumed scaled input
  void getDescaled(torch::Tensor & input) const;

protected:
  std::vector<Real> _mean;
  std::vector<Real> _stdev;
};

} // StochasticTools namespace

template <>
void dataStore(std::ostream & stream,
               StochasticToolsTorched::StandardizerTorched & standardizer,
               void * context);
template <>
void dataLoad(std::istream & stream,
              StochasticToolsTorched::StandardizerTorched & standardizer,
              void * context);
