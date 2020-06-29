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

/* Class standardizing data (centering and scaling)

 */
class Standardizer
{
public:
  Standardizer(){};

  /// Methods for setting mean and standard deviation directly
  void set(Real n);
  void set(Real mean, Real stdev);
  void set(Real mean, Real stdev, Real n);
  void set(std::vector<Real> mean, std::vector<Real> stdev);

  /// Methods for computing and setting mean and standard
  void computeSet(RealEigenMatrix input);

  /// Helper for dataStore
  void storeHelper(std::ostream & stream, void * context) const;

  RealEigenMatrix getStandardized(const RealEigenMatrix input) const;

  RealEigenMatrix getDestandardized(const RealEigenMatrix input) const;

  RealEigenMatrix getDescaled(const RealEigenMatrix input) const;

protected:
    std::vector<Real> _mean;
    std::vector<Real> _stdev;

};

} // namespace

template <>
void dataStore(std::ostream & stream,
               StochasticTools::Standardizer & standardizer,
               void * context);
template <>
void dataLoad(std::istream & stream,
              StochasticTools::Standardizer & standardizer,
              void * context);
