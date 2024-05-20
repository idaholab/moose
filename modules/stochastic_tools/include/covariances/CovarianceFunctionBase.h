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
#include "CovarianceInterface.h"

class CovarianceFunctionBase : public MooseObject, public CovarianceInterface
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

  virtual bool
  getTuningData(const std::string & name, unsigned int & size, Real & min, Real & max) const;

  /// New function that stores all the types of all the nested covariances
  void dependentCovarianceTypes(std::map<UserObjectName, std::string> name_type_map) const;

  /// Redirect dK/dhp for hyperparameter "hp"
  virtual bool computedKdhyper(RealEigenMatrix & dKdhp,
                               const RealEigenMatrix & x,
                               const std::string & hyper_param_name,
                               unsigned int ind) const;

  virtual bool isTunable(const std::string & name) const;

  unsigned int numOutputs() const { return _num_outputs; }

  std::unordered_map<std::string, Real> & hyperParamMapReal() { return _hp_map_real; }
  std::unordered_map<std::string, std::vector<Real>> & hyperParamMapVectorReal()
  {
    return _hp_map_vector_real;
  }

protected:
  const Real &
  addRealHyperParameter(const std::string & name, const Real value, const bool is_tunable);
  const std::vector<Real> & addVectorRealHyperParameter(const std::string & name,
                                                        const std::vector<Real> value,
                                                        const bool is_tunable);

  /// Map of real-valued hyperparameters
  std::unordered_map<std::string, Real> _hp_map_real;

  /// Map of vector-valued hyperparameters
  std::unordered_map<std::string, std::vector<Real>> _hp_map_vector_real;

  /// list of tunable hyper-parameters
  std::unordered_set<std::string> _tunable_hp;

  const unsigned int _num_outputs;

  const std::vector<UserObjectName> _dependent_covariance_names;

  std::vector<std::string> _dependent_covariance_types;

  std::vector<CovarianceFunctionBase *> _covariance_functions;
};
