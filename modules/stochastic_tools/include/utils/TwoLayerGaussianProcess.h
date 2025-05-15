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

/**
 * Utility class dedicated to hold structures and functions commont to
 * Gaussian Processes. It can be used to standardize parameters, manipulate
 * covariance data and compute additional stored matrices.
 */
class TwoLayerGaussianProcess
{
public:
  TwoLayerGaussianProcess();

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
  struct TGPOptimizerOptions
  {
    /// Default constructor
    TGPOptimizerOptions();
    /**
     * Construct a new TGPOptimizerOptions object using
     * input parameters that will control the optimization
     * @param show_every_nth_iteration To show the loss value at every n-th iteration, if set to 0,
     * nothing is displayed
     * @param num_iter The number of iterations we want in the optimization of the TGP
     * @param batch_size The number of samples in each batch
     * @param learning_rate The learning rate for parameter updates
     * @param b1 Tuning constant for the Adam algorithm
     * @param b2 Tuning constant for the Adam algorithm
     * @param eps Tuning constant for the Adam algorithm
     * @param lambda Tuning constant for the Adam algorithm
     */
    TGPOptimizerOptions(const bool show_every_nth_iteration = 1,
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
   * @param training_params The training parameter values (x values) for the
   *                        covariance matrix.
   * @param training_data The training data (y values) for the inversion of the
   *                      covariance matrix.
   * @param opts The optimizer options.
   */
  void setupCovarianceMatrix(const RealEigenMatrix & training_params,
                             const RealEigenMatrix & training_data,
                             const TGPOptimizerOptions & opts);

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

  // void sq_dist(const RealEigenMatrix &X1_in, RealEigenMatrix &D_out, const RealEigenMatrix &X2_in = RealEigenMatrix(0,0));

  /**
   * Parameter setting for MCMC sampling.
   * @param l Constant parameter from the paper.
   * @param u Constant parameter from the paper.
   * @param noise Noise level.
   * @param lengthscale_w Lengthscale for inner layer.
   * @param lengthscale_y Lengthscale for outer layer.
   * @param alpha Parameter for gamma distribution.
   * @param beta Parameter for gamma distribution.
   */
  struct Settings {
    Real l;
    Real u;
    struct {
      Real noise;
      Real lengthscale_w;
      Real lengthscale_y;
    } alpha, beta;
  };

  /**
   * Initialzed value for two layer GP hyperparameters.
   * @param w Hidden node.
   * @param lengthscale_w Lengthscale for inner layer.
   * @param lengthscale_y Lengthscale for outer layer.
   * @param noise Noise level.
   * @param scale Scale.
   */
  struct Initial {
    RealEigenMatrix w;
    RealEigenMatrix lengthscale_y;
    RealEigenMatrix lengthscale_w;
    Real noise;
    Real scale;
  };

  /**
   * Return value computed in the sampleNoise function.
   * @param noise Sampled noise level.
   * @param ll Log likelihood.
   */
  struct SampleNoiseResult {
    Real noise;
    Real ll;
  };

  /**
   * Return value computed in the sampleLengthscale function.
   * @param noise Sampled noise level.
   * @param ll Log likelihood.
   * @param scale Sampled scale.
   */
  struct SampleLengthscaleResult {
    Real lengthscale;
    Real ll;
    Real scale;
  };

  /**
   * Return value computed in the sampleW function.
   * @param w Sampled hidden node.
   * @param ll Log likelihood.
   */
  struct SampleWResult {
    RealEigenMatrix w;
    Real ll;
  };

  /**
   * Return value computed in the logl function.
   * @param logl Marginal log likelihood.
   * @param scale Scale.
   */
  struct LogLResult {
    Real logl;
    Real scale;
  };

  /**
   * Computes multivariate normal marginal log likelihood and scale.
   * @param outvec Observed outputs (response values).
   * @param x1 Input data 1.
   * @param x2 Input data 2.
   * @param noise Noise level.
   * @param lengthscale Lengthscale.
   * @param result Return value.
   * @param outer If the function is called for computing outermost layer.
   * @param scale If scale is needed to be computed.
   */
  void logl(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale, 
          LogLResult & result, bool outer=true, bool scale=false);

  /**
   * Samples noise level noise using MH algorithm.
   * @param outvec Observed outputs (response values).
   * @param x1 Input data 1.
   * @param x2 Input data 2.
   * @param g_t Noise level.
   * @param lengthscale Lengthscale.
   * @param settings parameter setting
   * @param ll_prev Log likelihood from the previous MCMC round.
   * @param result Return value.
   */
  void sampleNoise(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real g_t, const RealEigenMatrix lengthscale, 
              Settings & settings, Real ll_prev, SampleNoiseResult & result);

  /**
   * Samples lengthscale lengthscale using MH algorithm.
   * @param outvec Observed outputs (response values).
   * @param x1 Input data 1.
   * @param x2 Input data 2.
   * @param noise Noise level.
   * @param lengthscale_t Lengthscale.
   * @param i index for input data dimension.
   * @param settings parameter setting
   * @param ll_prev Log likelihood from the previous MCMC round.
   * @param result Return value.
   */
  void sampleLengthscale(const RealEigenMatrix & out_vec, const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale_t,
               unsigned int i, Real alpha, Real beta, Real l, Real u, SampleLengthscaleResult & result, Real ll_prev);

  /**
   * Samples from multivariate normal distribution.
   * @param mean Mean value.
   * @param cov Covariance matrix.
   * @param n_dim Dimension of data.
   * @param n_draw Number of draws from the distribution.
   * @param final_sample_matrix Sampled value.
   */
  void multiVariateNormalSampling(const RealEigenMatrix & mean,const RealEigenMatrix & cov, unsigned int n_dim, unsigned int n_draw, RealEigenMatrix & final_sample_matrix);

  /**
   * Samples lengthscale lengthscale using MH algorithm.
   * @param outvec Observed outputs (response values).
   * @param w_t Current values of the hidden nodes.
   * @param w1 Hidden node 1.
   * @param w2 Hidden node 2.
   * @param x1 Input data 1.
   * @param x2 Input data 2.
   * @param noise Noise level.
   * @param lengthscale_y Lengthscale for outer layer.
   * @param lengthscale_w Lengthscale for inner layer.
   * @param result Return value.
   * @param ll_prev Log likelihood from the previous MCMC round.
   * @param prior_mean Prior mean.
   */
  void sampleW(const RealEigenMatrix & out_vec, RealEigenMatrix & w_t, const RealEigenMatrix & w1, const RealEigenMatrix & w2, 
              const RealEigenMatrix & x1, const RealEigenMatrix & x2, Real noise, const RealEigenMatrix & lengthscale_y, const RealEigenMatrix & lengthscale_w,
              SampleWResult & result, Real ll_prev, const RealEigenMatrix & prior_mean);

  /**
   * Kernel function.
   * @param x1 Input data 1.
   * @param x2 Input data 2.
   * @param scale Scale.
   * @param lengthscale Lengthscale.
   * @param noise Noise level.
   * @param k Return value.
   */
  void squared_exponential_covariance(const RealEigenMatrix &x1, 
                  const RealEigenMatrix &x2, 
                  Real scale, 
                  const RealEigenMatrix &lengthscale, 
                  Real noise, 
                  RealEigenMatrix &k);

  /**
   * Predicts mean and covariance using Kriging interpolation.
   * @param y Observed outputs (response values).
   * @param x Input data.
   * @param x_new New input data.
   * @param lengthscale Lengthscale.
   * @param noise Noise level.
   * @param scale Scale.
   * @param cal_sigma If covariance needs to be computed.
   * @param prior_mean Prior mean for the observed input data.
   * @param prior_mean_new Prior mean for the new data points.
   * @param krig_mean Return mean.
   * @param krig_sigma Return covariance.
   */
  void krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
                                   const RealEigenMatrix & lengthscale, Real noise, Real scale, bool cal_sigma,
                                   const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new, 
                                   RealEigenMatrix & krig_mean, RealEigenMatrix & krig_sigma);


  /**
   * Sets up constant parameter.
   * @param settings Structure that stores parameter setting.
   */
  void check_settings(Settings & settings);

  // Tune hyperparameters using MCMC
  void tuneHyperParamsMcmc(const RealEigenMatrix & training_params,
                           const RealEigenMatrix & training_data);

  // Tune hyperparameters using Adam
  void tuneHyperParamsAdam(const RealEigenMatrix & training_params,
                           const RealEigenMatrix & training_data,
                           const TGPOptimizerOptions & opts);

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
  RealEigenMatrix & getNoise() { return _noise; }
  RealEigenMatrix & getLengthscaleY() { return _lengthscale_y; }
  RealEigenMatrix & getLengthscaleW() { return _lengthscale_w; }
  RealEigenMatrix & getScale() { return _scale; }
  std::vector<RealEigenMatrix> & getW() { return _w; }
  Real & getNmcmc() {return _nmcmc;}
  RealEigenMatrix & getX() { return _x; }
  RealEigenMatrix & getY() { return _y; }
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
  ///@}

protected:
  /// Covariance function object
  CovarianceFunctionBase * _covariance_function = nullptr;

  /// Contains tuning inforation. Index of hyperparam, size, and min/max bounds
  std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> _tuning_data;

  /// Number of tunable hyperparameters
  unsigned int _num_tunable;

  /// Type of covariance function used for this TGP
  std::string _covar_type;

  /// The name of the covariance function used in this TGP
  std::string _covar_name;

  /// The names of the covariance functions the used covariance function depends on
  std::vector<UserObjectName> _dependent_covar_names;

  /// The types of the covariance functions the used covariance function depends on
  std::map<UserObjectName, std::string> _dependent_covar_types;

  /// The number of outputs of the TGP
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

  RealEigenMatrix _noise;

  RealEigenMatrix _lengthscale_y;

  RealEigenMatrix _lengthscale_w;

  RealEigenMatrix _scale;

  std::vector<RealEigenMatrix> _w;

  Real _nmcmc;

  RealEigenMatrix _x;

  RealEigenMatrix _y;
};

} // StochasticTools namespac

// template <>
// void dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);
// template <>
// void dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context);

template <>
void dataStore(std::ostream & stream, StochasticTools::TwoLayerGaussianProcess & tgp_utils, void * context);
template <>
void dataLoad(std::istream & stream, StochasticTools::TwoLayerGaussianProcess & tgp_utils, void * context);
