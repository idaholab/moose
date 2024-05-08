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

  void loadHyperParamMap(const std::unordered_map<std::string, Real> & map,
                         const std::unordered_map<std::string, std::vector<Real>> & vec_map);

  void buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                          std::unordered_map<std::string, std::vector<Real>> & vec_map) const;

  virtual void getTuningData(std::string name, unsigned int & size, Real & min, Real & max) const;

  /// Redirect dK/dhp for hyperparameter "hp"
  virtual void computedKdhyper(RealEigenMatrix & dKdhp,
                               const RealEigenMatrix & x,
                               std::string hyper_param_name,
                               unsigned int ind) const;

  virtual bool isTunable(std::string name) const;

  std::unordered_map<std::string, Real> & hyperParamMapReal() { return _hp_map_real; }
  std::unordered_map<std::string, std::vector<Real>> & hyperParamMapVectorReal()
  {
    return _hp_map_vector_real;
  }

protected:
  const Real & addRealHyperParameter(const std::string & name, const Real value);
  const std::vector<Real> & addVectorRealHyperParameter(const std::string & name,
                                                        const std::vector<Real> value);

  /// Map of real-valued hyperparameters
  std::unordered_map<std::string, Real> _hp_map_real;

  /// Map of vector-valued hyperparameters
  std::unordered_map<std::string, std::vector<Real>> _hp_map_vector_real;

  /// list of tunable hyper-parameters
  std::unordered_set<std::string> _tunable_hp;
};
