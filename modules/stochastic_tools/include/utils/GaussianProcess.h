//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "GPLinkFunction.h"

namespace StochasticTools
{

/**
 * Utility class dedicated to hold structures and functions commont to
 * Gaussian Processes. It can be used to standardize parameters, manipulate
 * covariance data and compute additional stored matrices.
 */
class GaussianProcess
{
public:
  GaussianProcess();

  /**
   * Initializes the most important structures in the Gaussian Process: the
   * covariance function and a tuning map which is used if the user requires
   * parameter tuning.
   * @param covariance_function Pointer to the covariance function that
   *                            needs to be used for the Gaussian Process.
   * @param params_to_tune List of parameters which need to be tuned.
   * @param min List of lower bounds for the parameter tuning.
   * @param max List of upper bounds for parameter tuning.
   */
  void initialize(CovarianceFunctionBase * covariance_function,
                  const std::vector<std::string> & params_to_tune,
                  const std::vector<Real> & min = std::vector<Real>(),
                  const std::vector<Real> & max = std::vector<Real>());

  /// Structure containing the optimization options for
  /// hyperparameter-tuning
  struct GPOptimizerOptions
  {
    /// Default constructor
    GPOptimizerOptions();
    /**
     * Construct a new GPOptimizerOptions object using
     * input parameters that will control the optimization
     * @param show_every_nth_iteration To show the loss value at every n-th iteration, if set to 0,
     * nothing is displayed
     * @param num_iter The number of iterations we want in the optimization of the GP
     * @param batch_size The number of samples in each batch
     * @param learning_rate The learning rate for parameter updates
     * @param b1 Tuning constant for the Adam algorithm
     * @param b2 Tuning constant for the Adam algorithm
     * @param eps Tuning constant for the Adam algorithm
     * @param lambda Tuning constant for the Adam algorithm
     */
    GPOptimizerOptions(const bool show_every_nth_iteration = 1,
                       const unsigned int num_iter = 1000,
                       const unsigned int batch_size = 0,
                       const Real learning_rate = 1e-3,
                       const Real b1 = 0.9,
                       const Real b2 = 0.999,
                       const Real eps = 1e-7,
                       const Real lambda = 0.0);

    /// Switch to enable verbose output for parameter tuning at every n-th iteration
    const unsigned int show_every_nth_iteration = false;
    /// The number of iterations for Adam optimizer
    const unsigned int num_iter = 1000;
    /// The batch isize for Adam optimizer
    const unsigned int batch_size = 0;
    /// The learning rate for Adam optimizer
    const Real learning_rate = 1e-3;
    /// Tuning parameter from the paper
    const Real b1 = 0.9;
    /// Tuning parameter from the paper
    const Real b2 = 0.999;
    /// Tuning parameter from the paper
    const Real eps = 1e-7;
    /// Tuning parameter from the paper
    const Real lambda = 0.0;
  };
  /**
   * Sets up the covariance matrix given data and optimization options.
   * If virtual derivative observations have been configured via setDerivativeConstraints(),
   * builds the full augmented covariance matrix incorporating those observations.
   * @param training_params The training parameter values (x values, already standardized).
   * @param training_data The training data (y values, link-transformed and standardized).
   * @param opts The optimizer options.
   */
  void setupCovarianceMatrix(const RealEigenMatrix & training_params,
                             const RealEigenMatrix & training_data,
                             const GPOptimizerOptions & opts);

  /**
   * Configure the output link function. Call before setupCovarianceMatrix.
   * @param type  Link function type (Identity, Log, or Logit)
   * @param lb    Lower bound (required for Log and Logit)
   * @param ub    Upper bound (required for Logit)
   */
  void setLinkFunction(GPLinkFunctionType type, Real lb = 0.0, Real ub = 1.0);

  /**
   * Apply the forward link function to all entries of a data matrix (in-place).
   * Validates that all values respect the link function's domain.
   */
  void applyLinkTransform(RealEigenMatrix & data) const;

  /**
   * Configure virtual derivative observations for monotonicity/derivative constraints.
   * Call before setupCovarianceMatrix. The virtual_params must be in the same
   * standardized space as the training parameters.
   * @param virtual_params   Locations of virtual observations (n_virt x n_dims), standardized
   * @param deriv_dims       Derivative dimension for each virtual observation
   * @param target_value     Target derivative value (0 = zero-derivative constraint,
   *                         positive = increasing, negative = decreasing)
   * @param noise_variance   Noise variance added to the derivative covariance diagonal
   */
  void setDerivativeConstraints(const RealEigenMatrix & virtual_params,
                                const std::vector<unsigned int> & deriv_dims,
                                Real target_value,
                                Real noise_variance);

  /**
   * Configure penalty constraints on the predicted mean at specified points.
   * These are evaluated and added to the NLML loss and its gradient during Adam.
   * The points and bounds must be in the standardized GP output space (post-link,
   * post-standardize). Call after standardization is set up.
   * @param penalty_points_std  Constraint point locations in standardized param space (n_c x d)
   * @param lower_bounds_std    Lower bounds in standardized output space (use -inf to disable)
   * @param upper_bounds_std    Upper bounds in standardized output space (use +inf to disable)
   * @param weight              Penalty weight lambda
   */
  void setPenaltyConstraints(const RealEigenMatrix & penalty_points_std,
                             const std::vector<Real> & lower_bounds_std,
                             const std::vector<Real> & upper_bounds_std,
                             Real weight);

  /// Apply forward link function to a scalar
  Real applyLink(Real y) const;
  /// Apply inverse link function to a scalar
  Real applyInvLink(Real z) const;
  /// Derivative of the inverse link at z, for delta-method uncertainty propagation
  Real invLinkDeriv(Real z) const;
  /// log|g'(y)| for the Jacobian correction to NLML reporting
  Real logLinkJacobian(Real y) const;
  /// True if a non-identity link function is set
  bool hasLinkFunction() const { return _link_type != GPLinkFunctionType::Identity; }

  /// @{ Accessors for link function parameters (for serialization)
  GPLinkFunctionType & linkType() { return _link_type; }
  Real & linkLb() { return _link_lb; }
  Real & linkUb() { return _link_ub; }
  const GPLinkFunctionType & linkType() const { return _link_type; }
  const Real & linkLb() const { return _link_lb; }
  const Real & linkUb() const { return _link_ub; }
  /// @}

  /// @{ Accessors for virtual derivative observations (for serialization and prediction)
  RealEigenMatrix & virtualParams() { return _virtual_params; }
  std::vector<unsigned int> & virtualDerivDims() { return _virtual_deriv_dims; }
  Real & virtualNoise() { return _virtual_noise; }
  Real & virtualTarget() { return _virtual_target; }
  const RealEigenMatrix & virtualParams() const { return _virtual_params; }
  const std::vector<unsigned int> & virtualDerivDims() const { return _virtual_deriv_dims; }
  const Real & virtualNoise() const { return _virtual_noise; }
  bool hasDerivativeConstraints() const { return _virtual_params.rows() > 0; }
  /// @}

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
  void linkCovarianceFunction(CovarianceFunctionBase * covariance_function);

  /**
   * Sets up the tuning map which is used if the user requires parameter tuning.
   * @param params_to_tune List of parameters which need to be tuned.
   * @param min List of lower bounds for the parameter tuning.
   * @param max List of upper bounds for parameter tuning.
   */
  void generateTuningMap(const std::vector<std::string> & params_to_tune,
                         const std::vector<Real> & min = std::vector<Real>(),
                         const std::vector<Real> & max = std::vector<Real>());

  /**
   * Standardizes the vector of input parameters (x values).
   * @param parameters The vector/matrix of input data.
   * @param keep_moments If previously computed or new moments are to be used.
   */
  void standardizeParameters(RealEigenMatrix & parameters, bool keep_moments = false);

  /**
   * Standardizes the vector of responses (y values).
   * @param data The vector/matrix of input data.
   * @param keep_moments If previously computed or new moments are to be used.
   */
  void standardizeData(RealEigenMatrix & data, bool keep_moments = false);

  // Tune hyperparameters using Adam
  void tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                           const RealEigenMatrix & training_data,
                           const GPOptimizerOptions & opts);

  // Computes the loss function
  Real getLoss(RealEigenMatrix & inputs, RealEigenMatrix & outputs);

  // Computes Gradient of the loss function
  std::vector<Real> getGradient(RealEigenMatrix & inputs) const;

  /// Function used to convert the hyperparameter maps in this object to
  /// vectors
  void mapToVec(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      const std::unordered_map<std::string, Real> & scalar_map,
      const std::unordered_map<std::string, std::vector<Real>> & vector_map,
      std::vector<Real> & vec) const;

  /// Function used to convert the vectors back to hyperparameter maps
  void vecToMap(
      const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
          tuning_data,
      std::unordered_map<std::string, Real> & scalar_map,
      std::unordered_map<std::string, std::vector<Real>> & vector_map,
      const std::vector<Real> & vec) const;

  /// @{
  /**
   * Get constant reference to the contained structures
   */
  const StochasticTools::Standardizer & getParamStandardizer() const { return _param_standardizer; }
  const StochasticTools::Standardizer & getDataStandardizer() const { return _data_standardizer; }
  const RealEigenMatrix & getK() const { return _K; }
  const RealEigenMatrix & getKResultsSolve() const { return _K_results_solve; }
  const Eigen::LLT<RealEigenMatrix> & getKCholeskyDecomp() const { return _K_cho_decomp; }
  const CovarianceFunctionBase & getCovarFunction() const { return *_covariance_function; }
  const CovarianceFunctionBase * getCovarFunctionPtr() const { return _covariance_function; }
  const std::string & getCovarType() const { return _covar_type; }
  const std::string & getCovarName() const { return _covar_name; }
  const std::vector<UserObjectName> & getDependentCovarNames() const
  {
    return _dependent_covar_names;
  }
  const std::map<UserObjectName, std::string> & getDependentCovarTypes() const
  {
    return _dependent_covar_types;
  }
  const unsigned int & getCovarNumOutputs() const { return _num_outputs; }
  const unsigned int & getNumTunableParams() const { return _num_tunable; }
  const std::unordered_map<std::string, Real> & getHyperParamMap() const { return _hyperparam_map; }
  const std::unordered_map<std::string, std::vector<Real>> & getHyperParamVectorMap() const
  {
    return _hyperparam_vec_map;
  }
  const std::vector<Real> & getLengthScales() const { return _length_scales; }
  ///@}

  /// @{
  /**
   * Get non-constant reference to the contained structures (if they need to be modified from the
   * utside)
   */
  StochasticTools::Standardizer & paramStandardizer() { return _param_standardizer; }
  StochasticTools::Standardizer & dataStandardizer() { return _data_standardizer; }
  RealEigenMatrix & K() { return _K; }
  RealEigenMatrix & KResultsSolve() { return _K_results_solve; }
  Eigen::LLT<RealEigenMatrix> & KCholeskyDecomp() { return _K_cho_decomp; }
  CovarianceFunctionBase * covarFunctionPtr() { return _covariance_function; }
  CovarianceFunctionBase & covarFunction() { return *_covariance_function; }
  std::string & covarType() { return _covar_type; }
  std::string & covarName() { return _covar_name; }
  std::map<UserObjectName, std::string> & dependentCovarTypes() { return _dependent_covar_types; }
  std::vector<UserObjectName> & dependentCovarNames() { return _dependent_covar_names; }
  unsigned int & covarNumOutputs() { return _num_outputs; }
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> & tuningData()
  {
    return _tuning_data;
  }
  std::unordered_map<std::string, Real> & hyperparamMap() { return _hyperparam_map; }
  std::unordered_map<std::string, std::vector<Real>> & hyperparamVectorMap()
  {
    return _hyperparam_vec_map;
  }
  std::vector<Real> & lengthScales() { return _length_scales; }
  ///@}

protected:
  /// Covariance function object
  CovarianceFunctionBase * _covariance_function = nullptr;

  /// Contains tuning inforation. Index of hyperparam, size, and min/max bounds
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> _tuning_data;

  /// Number of tunable hyperparameters
  unsigned int _num_tunable;

  /// Type of covariance function used for this GP
  std::string _covar_type;

  /// The name of the covariance function used in this GP
  std::string _covar_name;

  /// The names of the covariance functions the used covariance function depends on
  std::vector<UserObjectName> _dependent_covar_names;

  /// The types of the covariance functions the used covariance function depends on
  std::map<UserObjectName, std::string> _dependent_covar_types;

  /// The number of outputs of the GP
  unsigned int _num_outputs;

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

  /// The batch size for Adam optimization
  unsigned int _batch_size;

  /// To return the GP length scales for active learning
  std::vector<Real> _length_scales;

  // ---- Link function (serialized) ----
  /// Link function type tag (stored as enum, serialized as int)
  GPLinkFunctionType _link_type = GPLinkFunctionType::Identity;
  /// Lower bound for Log/Logit link
  Real _link_lb = 0.0;
  /// Upper bound for Logit link
  Real _link_ub = 1.0;

  // ---- Virtual derivative observations (serialized; needed for prediction) ----
  /// Locations of virtual observations in standardized parameter space (n_virt x n_dims)
  RealEigenMatrix _virtual_params;
  /// Derivative dimension for each virtual observation
  std::vector<unsigned int> _virtual_deriv_dims;
  /// Noise variance added to the derivative-derivative diagonal blocks of K_aug
  Real _virtual_noise = 1e-6;
  /// Target derivative value for virtual observations
  Real _virtual_target = 0.0;

  // ---- Penalty constraints (not serialized; only used during Adam tuning) ----
  /// Constraint point locations in standardized parameter space (n_c x n_dims)
  RealEigenMatrix _penalty_points_std;
  /// Lower bounds in standardized output space (-inf disables lower constraint)
  std::vector<Real> _penalty_lower_std;
  /// Upper bounds in standardized output space (+inf disables upper constraint)
  std::vector<Real> _penalty_upper_std;
  /// Penalty weight lambda
  Real _penalty_weight = 0.0;
};

} // StochasticTools namespac

template <>
void dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);
template <>
void dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);

template <>
void dataStore(std::ostream & stream, StochasticTools::GaussianProcess & gp_utils, void * context);
template <>
void dataLoad(std::istream & stream, StochasticTools::GaussianProcess & gp_utils, void * context);
