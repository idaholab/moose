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

namespace StochasticTools
{

/// Class for standardizing data (centering and scaling)

class Standardizer
{
public:
  Standardizer() = default;

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
  void computeSet(const RealEigenMatrix & input);

  /// Returns the standardized (centered and scaled) of the provided input
  void getStandardized(RealEigenMatrix & input) const;

  /// De-standardizes (de-centered and de-scaled) the assumed standardized input
  void getDestandardized(RealEigenMatrix & input) const;

  /// De-scales the assumed scaled input
  void getDescaled(RealEigenMatrix & input) const;

  /// Scales the assumed de-scaled input
  void getScaled(RealEigenMatrix & input) const;

protected:
  std::vector<Real> _mean;
  std::vector<Real> _stdev;
};

// Template implementation of store/load functions
template <typename Context>
void
dataStore(std::ostream & stream, Standardizer & standardizer, Context context)
{
  unsigned int n = standardizer.getMean().size();
  ::dataStore(stream, n, context);
  for (unsigned int ii = 0; ii < n; ++ii)
    ::dataStore(stream, standardizer.getMean()[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    ::dataStore(stream, standardizer.getStdDev()[ii], context);
}

template <typename Context>
void
dataLoad(std::istream & stream, Standardizer & standardizer, Context context)
{
  unsigned int n;
  ::dataLoad(stream, n, context);
  std::vector<Real> mean(n);
  std::vector<Real> stdev(n);
  for (unsigned int ii = 0; ii < n; ++ii)
    ::dataLoad(stream, mean[ii], context);
  for (unsigned int ii = 0; ii < n; ++ii)
    ::dataLoad(stream, stdev[ii], context);
  standardizer.set(mean, stdev);
}

} // StochasticTools namespace
