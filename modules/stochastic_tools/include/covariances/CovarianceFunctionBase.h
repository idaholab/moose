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

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual void computeCovarianceMatrix(RealEigenMatrix & K,
                                       const RealEigenMatrix & x,
                                       const RealEigenMatrix & xp,
                                       const bool is_self_covariance) const = 0;

  /// Used for outputting Hyper-parameter settings
  virtual void
  buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                     std::unordered_map<std::string, std::vector<Real>> & vec_map) const = 0;

  /// Used for outputting Hyper-parameter settings for use in surrogate
  virtual void loadHyperParamMap(std::unordered_map<std::string, Real> & map,
                                 std::unordered_map<std::string, std::vector<Real>> & vec_map) = 0;

  virtual void
  getTuningData(std::string name, unsigned int & size, Real & min, Real & max) const = 0;

  /// Redirect dK/dhp for hyperparameter "hp"
  virtual void computedKdhyper(RealEigenMatrix & dKdhp,
                               const RealEigenMatrix & x,
                               std::string hyper_param_name,
                               unsigned int ind) const;

  virtual bool isTunable(std::string name) const;

protected:
  /// list of tunable hyper-parameters
  std::unordered_set<std::string> _tunable_hp;
};
