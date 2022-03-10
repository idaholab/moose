//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Standardizer.h"
#include <Eigen/Dense>

#include "CovarianceFunctionBase.h"

namespace StochasticTools
{

class GaussianProcessUtils
{
public:
  GaussianProcessUtils();

  void setupStoredMatrices(const RealEigenMatrix & input);

  void linkCovarianceFunction(const InputParameters & parameters, const UserObjectName & name);

  void generateTuningMap(const std::vector<std::string> params_to_tune,
                         std::vector<Real> min = std::vector<Real>(),
                         std::vector<Real> max = std::vector<Real>());

  void standardizeParameters(RealEigenMatrix & parameters, bool keep_moments = false);
  void standardizeData(RealEigenMatrix & data, bool keep_moments = false);

  PetscErrorCode tuneHyperParamsTAO(const RealEigenMatrix & training_params,
                                    const RealEigenMatrix & training_data,
                                    std::string tao_options = "",
                                    bool show_tao = false);

  PetscErrorCode formInitialGuessTAO(Vec theta_vec);

  void buildHyperParamBoundsTAO(libMesh::PetscVector<Number> & theta_l,
                                libMesh::PetscVector<Number> & theta_u) const;

  // Wrapper for PETSc function callback
  static PetscErrorCode
  formFunctionGradientWrapper(Tao tao, Vec theta, PetscReal * f, Vec Grad, void * ptr);

  // Computes Gradient of the loss function
  void formFunctionGradient(Tao tao, Vec theta, PetscReal * f, Vec Grad);

  void mapToPetscVec(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      const std::unordered_map<std::string, Real> & scalar_map,
      const std::unordered_map<std::string, std::vector<Real>> & vector_map,
      libMesh::PetscVector<Number> & petsc_vec);

  void petscVecToMap(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      std::unordered_map<std::string, Real> & scalar_map,
      std::unordered_map<std::string, std::vector<Real>> & vector_map,
      const libMesh::PetscVector<Number> & petsc_vec);

  const StochasticTools::Standardizer & getParamStandardizer() const { return _param_standardizer; }
  const StochasticTools::Standardizer & getDataStandardizer() const { return _data_standardizer; }
  const RealEigenMatrix & getK() const { return _K; }
  const RealEigenMatrix & getKResultsSolve() const { return _K_results_solve; }
  const Eigen::LLT<RealEigenMatrix> & getKCholeskyDecomp() const { return _K_cho_decomp; }
  const CovarianceFunctionBase & getCovarFunction() const { return *_covariance_function; }
  const std::string & getCovarType() const { return _covar_type; }
  const unsigned int & getNumTunableParams() const { return _num_tunable; }
  const std::unordered_map<std::string, Real> & getHyperParamMap() const { return _hyperparam_map; }
  const std::unordered_map<std::string, std::vector<Real>> & getHyperParamVectorMap() const
  {
    return _hyperparam_vec_map;
  }

  StochasticTools::Standardizer & paramStandardizer() { return _param_standardizer; }
  StochasticTools::Standardizer & dataStandardizer() { return _data_standardizer; }
  RealEigenMatrix & K() { return _K; }
  RealEigenMatrix & KResultsSolve() { return _K_results_solve; }
  Eigen::LLT<RealEigenMatrix> & KCholeskyDecomp() { return _K_cho_decomp; }
  CovarianceFunctionBase * covarFunctionPtr() { return _covariance_function; }
  CovarianceFunctionBase & covarFunction() { return *_covariance_function; }
  std::string & covarType() { return _covar_type; }
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> & tuningData()
  {
    return _tuning_data;
  }
  std::unordered_map<std::string, Real> & hyperparamMap() { return _hyperparam_map; }
  std::unordered_map<std::string, std::vector<Real>> & hyperparamVectorMap()
  {
    return _hyperparam_vec_map;
  }

protected:
  /// Covariance function object
  CovarianceFunctionBase * _covariance_function = nullptr;

  /// Contains tuning inforation. Index of hyperparam, size, and min/max bounds
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> _tuning_data;

  /// Number of tunable hyperparameters
  unsigned int _num_tunable;

  /// Type of covariance function used for this surrogate
  std::string _covar_type;

  /// Tao Communicator
  Parallel::Communicator _tao_comm;

  /// Scalar hyperparameters. Stored for use in surrogate
  std::unordered_map<std::string, Real> _hyperparam_map;

  /// Vector hyperparameters. Stored for use in surrogate
  std::unordered_map<std::string, std::vector<Real>> _hyperparam_vec_map;

  /// Standardizer for use with params (x)
  StochasticTools::Standardizer _param_standardizer;

  /// Standardizer for use with data (y)
  StochasticTools::Standardizer _data_standardizer;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  RealEigenMatrix _K;

  /// A solve of Ax=b via Cholesky.
  RealEigenMatrix _K_results_solve;

  /// Cholesky decomposition Eigen object
  Eigen::LLT<RealEigenMatrix> _K_cho_decomp;

  /// Paramaters (x) used for training, along with statistics
  const RealEigenMatrix * _training_params;

  /// Data (y) used for training
  const RealEigenMatrix * _training_data;
};

} // StochasticTools namespac

template <>
void dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);
template <>
void dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);

template <>
void
dataStore(std::ostream & stream, StochasticTools::GaussianProcessUtils & gp_utils, void * context);
template <>
void
dataLoad(std::istream & stream, StochasticTools::GaussianProcessUtils & gp_utils, void * context);
