//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  /// Methods for computing and setting mean and standard
  void computeSet(const RealEigenMatrix & input);

  /// Helper for dataStore
  void storeHelper(std::ostream & stream, void * context) const;

  /// Returns the standardized (centered and scaled) of the provided input
  void getStandardized(RealEigenMatrix & input) const;

  /// De-standardizes (de-centered and de-scaled) the assumed standardized input
  void getDestandardized(RealEigenMatrix & input) const;

  /// De-scales the assumed scaled input
  void getDescaled(RealEigenMatrix & input) const;

protected:
  std::vector<Real> _mean;
  std::vector<Real> _stdev;
};

} // StochasticTools namespace

template <>
void dataStore(std::ostream & stream, StochasticTools::Standardizer & standardizer, void * context);
template <>
void dataLoad(std::istream & stream, StochasticTools::Standardizer & standardizer, void * context);
