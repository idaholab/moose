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
#include "OutputCovarianceBase.h"

namespace StochasticTools
{

/**
 * Utility class dedicated to hold structures and functions common to
 * Multi Output Gaussian Processes. It can be used to standardize parameters, manipulate
 * covariance data and compute additional stored matrices.
 */
class MultiOutputGaussianProcess
{
public:
  MultiOutputGaussianProcess() {}

  /**
   * Initializes the most important structures in the Gaussian Process: the
   * covariance function and a tuning map which is used if the user requires
   * parameter tuning.
   * @param covariance_function Pointer to the covariance function that
   *                            needs to be used for the Gaussian Process.
   */
  void initialize(OutputCovarianceBase * output_covariance,
                  CovarianceFunctionBase * covariance_function,
                  const std::vector<std::string> params_to_tune,
                  std::vector<Real> min = std::vector<Real>(),
                  std::vector<Real> max = std::vector<Real>());

  /// Structure containing the optimization options for
  /// hyperparameter-tuning
  struct GPOptimizerOptions
  {
    /// Default constructor
    GPOptimizerOptions();
    /// Construct using user-input
    GPOptimizerOptions(const bool inp_show_optimization_details,
                       const unsigned int inp_iter = 1000,
                       const unsigned int inp_batch_size = 0,
                       const Real inp_learning_rate = 1e-3);

    /// Switch to enable verbose output for parameter tuning
    bool show_optimization_details = false;
    /// The number of iterations for Adam optimizer
    unsigned int iter = 1000;
    /// The batch isize for Adam optimizer
    unsigned int batch_size = 0;
    /// The learning rate for Adam optimizer
    Real learning_rate = 1e-3;
  };
  /**
   * Sets up the covariance matrix given data and optimization options.
   * @param training_params The training parameter values (x values) for the
   *                        covariance matrix.
   * @param training_data The training data (y values) for the inversion of the
   *                      covariance matrix.
   * @param opts The optimizer options.
   */
  void setupCovarianceMatrix(const RealEigenMatrix & training_params,
                             const RealEigenMatrix & training_data,
                             const GPOptimizerOptions & opts);

  /**
   * Sets up the Cholesky decomposition and inverse action of the covariance matrix.
   * @param input The vector/matrix which right multiples the inverse of the covariance matrix.
   */
  void setupStoredMatrices(const RealEigenMatrix & input);

  /**
   * Finds and links the covariance function to this object. Used mainly in the
   * covariance data action.
   * @param covariance_function Pointer to the covariance function that
   *                            needs to be used for the Gaussian Process.
   */
  void linkCovarianceFunction(OutputCovarianceBase * output_covariance,
                              CovarianceFunctionBase * covariance_function);

  /**
   * Sets up the tuning map which is used if the user requires parameter tuning.
   * @param params_to_tune List of parameters which need to be tuned.
   * @param min List of lower bounds for the parameter tuning.
   * @param max List of upper bounds for parameter tuning.
   */
  void generateTuningMap(const std::vector<std::string> params_to_tune,
                         std::vector<Real> min = std::vector<Real>(),
                         std::vector<Real> max = std::vector<Real>());

  /**
   * Standardizes the vector of input parameters (x values).
   * @param parameters The vector/matrix of input data.
   */
  void standardizeParameters(RealEigenMatrix & parameters);

  /**
   * Standardizes the vector of responses (y values).
   * @param data The vector/matrix of input data.
   */
  void standardizeData(RealEigenMatrix & data);

  // Tune hyperparameters using Adam
  void tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                           const RealEigenMatrix & training_data,
                           unsigned int iter,
                           const unsigned int & batch_size,
                           const Real & learning_rate,
                           const bool & verbose);

  // Computes the loss function for Adam usage
  Real getLoss(RealEigenMatrix & inputs, RealEigenMatrix & outputs);

  // Computes Gradient of the loss function for Adam usage
  std::vector<Real> getGradient(RealEigenMatrix & inputs); // , RealEigenMatrix & outputs

  /// Function used to convert the hyperparameter maps in this object to
  /// Petsc vectors
  void mapToPetscVec(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      const std::unordered_map<std::string, Real> & scalar_map,
      const std::unordered_map<std::string, std::vector<Real>> & vector_map,
      libMesh::PetscVector<Number> & petsc_vec);

  /// Function used to convert the PETSc vectors back to hyperparameter maps
  void petscVecToMap(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      std::unordered_map<std::string, Real> & scalar_map,
      std::unordered_map<std::string, std::vector<Real>> & vector_map,
      const libMesh::PetscVector<Number> & petsc_vec);

  /// @{
  /**
   * Get constant reference to the contained structures
   */
  const StochasticTools::Standardizer & getParamStandardizer() const { return _param_standardizer; }
  const StochasticTools::Standardizer & getDataStandardizer() const { return _data_standardizer; }
  const RealEigenMatrix & getK() const { return _K; }
  const RealEigenMatrix & getB() const { return _B; }
  const RealEigenMatrix & getKappa() const { return _kappa; }
  const std::vector<Real> & getLatent() const { return _latent; }
  const RealEigenMatrix & getKappaResultsSolve() const { return _kappa_results_solve; }
  const Eigen::LDLT<RealEigenMatrix> & getKappaCholeskyDecomp() const { return _kappa_cho_decomp; }
  const RealEigenMatrix & getBatchInputs() const { return _batch_inputs; }
  const RealEigenMatrix & getBatchOutputs() const { return _batch_outputs; }
  const CovarianceFunctionBase & getCovarFunction() const { return *_covariance_function; }
  const CovarianceFunctionBase * getCovarFunctionPtr() const { return _covariance_function; }
  const OutputCovarianceBase & getOutputCovar() const { return *_output_covariance; }
  const OutputCovarianceBase * getOutputCovarPtr() const { return _output_covariance; }
  const std::string & getCovarType() const { return _covar_type; }
  const std::string & getOutputCovarType() const { return _output_covar_type; }
  const unsigned int & getNumTunableParamsInp() const { return _num_tunable_inp; }
  const unsigned int & getNumTunableParamsOut() const { return _num_tunable_out; }
  const std::unordered_map<std::string, Real> & getHyperParamMap() const { return _hyperparam_map; }
  const std::unordered_map<std::string, std::vector<Real>> & getHyperParamVectorMap() const
  {
    return _hyperparam_vec_map;
  }
  ///@}

  /// @{
  /**
   * Get non-constant reference to the contained structures (if they need to be modified from the
   * utside)
   */
  StochasticTools::Standardizer & paramStandardizer() { return _param_standardizer; }
  StochasticTools::Standardizer & dataStandardizer() { return _data_standardizer; }
  RealEigenMatrix & K() { return _K; }
  RealEigenMatrix & B() { return _B; }
  RealEigenMatrix & kappa() { return _kappa; }
  std::vector<Real> & latent() { return _latent; }
  RealEigenMatrix & kappaResultsSolve() { return _kappa_results_solve; }
  Eigen::LDLT<RealEigenMatrix> & kappaCholeskyDecomp() { return _kappa_cho_decomp; }
  RealEigenMatrix & batchInputs() { return _batch_inputs; }
  RealEigenMatrix & batchOutputs() { return _batch_outputs; }
  CovarianceFunctionBase * covarFunctionPtr() { return _covariance_function; }
  CovarianceFunctionBase & covarFunction() { return *_covariance_function; }
  OutputCovarianceBase * outputCovarPtr() { return _output_covariance; }
  OutputCovarianceBase & outputCovar() { return *_output_covariance; }
  std::string & covarType() { return _covar_type; }
  std::string & outputCovarType() { return _output_covar_type; }
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> & tuningData()
  {
    return _tuning_data;
  }
  std::unordered_map<std::string, Real> & hyperparamMap() { return _hyperparam_map; }
  std::unordered_map<std::string, std::vector<Real>> & hyperparamVectorMap()
  {
    return _hyperparam_vec_map;
  }
  ///@}

protected:
  /// Covariance function object
  CovarianceFunctionBase * _covariance_function = nullptr;

  /// Output covariance object
  OutputCovarianceBase * _output_covariance = nullptr;

  /// Contains tuning inforation. Index of hyperparam, size, and min/max bounds
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> _tuning_data;

  /// Number of tunable hyperparameters (input covariance)
  unsigned int _num_tunable_inp;

  /// Number of tunable hyperparameters (output covariance)
  unsigned int _num_tunable_out;

  /// Type of covariance function used for this surrogate
  std::string _covar_type;

  /// Type of output covariance used for this surrogate
  std::string _output_covar_type;

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

  /// An _n_output by _n_output covariance matrix constructed from the selected output kernel
  RealEigenMatrix _B;

  /// An _n_output*_n_sample by _n_output*_n_sample covariance matrix
  RealEigenMatrix _kappa;

  /// A vector of latent params to capture output covariances
  std::vector<Real> _latent;

  /// A solve of Ax=b via Cholesky.
  RealEigenMatrix _kappa_results_solve;

  /// Cholesky decomposition Eigen object
  Eigen::LDLT<RealEigenMatrix> _kappa_cho_decomp;

  /// Paramaters (x) used for training, along with statistics
  const RealEigenMatrix * _training_params;

  /// Data (y) used for training
  const RealEigenMatrix * _training_data;

  /// The batch size for Adam optimization
  unsigned int _batch_size;

  /// The random batch of inputs
  RealEigenMatrix _batch_inputs;

  /// The random batch of outputs
  RealEigenMatrix _batch_outputs;
};

} // StochasticTools namespace

template <>
void dataStore(std::ostream & stream,
               StochasticTools::MultiOutputGaussianProcess & gp_utils,
               void * context);
template <>
void dataLoad(std::istream & stream,
              StochasticTools::MultiOutputGaussianProcess & gp_utils,
              void * context);
