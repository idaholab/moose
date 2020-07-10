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
  virtual void
  buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                     std::unordered_map<std::string, std::vector<Real>> & vec_map) const = 0;
};
