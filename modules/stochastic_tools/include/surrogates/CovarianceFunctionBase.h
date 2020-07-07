//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"

class CovarianceFunctionBase : public MooseObject
{
public:
  static InputParameters validParams();
  CovarianceFunctionBase(const InputParameters & parameters);
  // CovarianceFunctionBase(const std::vector<Real> & length_factor,
  //                      const Real & sigma_f_squared,
  //                      const Real & sigma_n_squared);

  // CovarianceFunctionBase(const std::vector<std::vector<Real>> & /*vec*/){};

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual RealEigenMatrix computeCovarianceMatrix(const RealEigenMatrix & x,
                                                  const RealEigenMatrix & xp,
                                                  const bool is_self_covariance) const = 0;

  /// Used for outputting Hyper-parameter settings
  virtual void getHyperParameters(std::vector<std::vector<Real>> & vec) const = 0;

protected:
  /// This needs to be declared first, before all other vairables that would be stored in it
  std::vector<std::vector<Real>> _hyperparams;

  /// lengh factor (\ell) for the kernel, in vector form for multiple parameters
  std::vector<Real> _length_factor;

  /// signal variance (\sigma_f^2)
  Real _sigma_f_squared;

  /// noise variance (\sigma_n^2)
  Real _sigma_n_squared;
};
