//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

/**
 * Base class for covariance functions that are used in Gaussian Processes
 */
class CovarianceFunctionBase : public MooseObject, public CovarianceInterface
{
public:
  static InputParameters validParams();
  CovarianceFunctionBase(const InputParameters & parameters);

  /// Generates the Covariance Matrix given two sets of points in the parameter space
  /// @param K Reference to a matrix which should be populated by the covariance entries
  /// @param x Reference to the first set of points
  /// @param xp Reference to the second set of points
  /// @param is_self_covariance Switch to enable adding the noise variance to the diagonal of the covariance matrix
  virtual void computeCovarianceMatrix(RealEigenMatrix & K,
                                       const RealEigenMatrix & x,
                                       const RealEigenMatrix & xp,
                                       const bool is_self_covariance) const = 0;

  /// Load some hyperparameters into the local maps contained in this object.
  /// @param map Input map of scalar hyperparameters
  /// @param vec_map Input map of vector hyperparameters
  void loadHyperParamMap(const std::unordered_map<std::string, Real> & map,
                         const std::unordered_map<std::string, std::vector<Real>> & vec_map);

  /// Populates the input maps with the owned hyperparameters.
  /// @param map Map of scalar hyperparameters that should be populated
  /// @param vec_map Map of vector hyperparameters that should be populated
  void buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                          std::unordered_map<std::string, std::vector<Real>> & vec_map) const;

  /// Get the default minimum and maximum and size of a hyperparameter.
  /// Returns false is the parameter has not been found in this covariance object.
  /// @param name The name of the hyperparameter
  /// @param size Reference to an unsigned int that will contain the size of the
  ///             hyperparameter (will be populated with 1 if it is scalar)
  /// @param min Reference to a number which will be populated by the maximum allowed value of the hyperparameter
  /// @param max Reference to a number which will be populated by the minimum allowed value of the hyperparameter
  virtual bool
  getTuningData(const std::string & name, unsigned int & size, Real & min, Real & max) const;

  /// Populate a map with the names and types of the dependent covariance functions
  /// @param name_type_map Reference to the map which should be populated
  void dependentCovarianceTypes(std::map<UserObjectName, std::string> & name_type_map) const;

  /// Get the names of the dependent covariances
  const std::vector<UserObjectName> & dependentCovarianceNames() const
  {
    return _dependent_covariance_names;
  }

  /// Redirect dK/dhp for hyperparameter "hp".
  /// Returns false is the parameter has not been found in this covariance object.
  /// @param dKdhp The matrix which should be populated with the derivatives
  /// @param x The input vector for which the derivatives of the covariance matrix
  ///          is computed
  /// @param hyper_param_name The name of the hyperparameter
  /// @param ind The index within the hyperparameter. 0 if it is a scalar parameter.
  ///            If it is a vector parameter, it should be the index within the vector.
  virtual bool computedKdhyper(RealEigenMatrix & dKdhp,
                               const RealEigenMatrix & x,
                               const std::string & hyper_param_name,
                               unsigned int ind) const;

  /**
   * Compute the cross-covariance between function values and derivatives:
   *   K_fd[i,j] = Cov[f(x_i), df(xd_j)/dx'_{dim}] = dK(x_i, xd_j)/dx'_{dim}
   * For SE kernel: K_fd[i,j] = K(x_i,xd_j) * (x_{i,dim} - xd_{j,dim}) / ell_dim^2
   * Default implementation calls mooseError; override in kernels that support
   * derivative observations.
   * @param K_fd  Output matrix of size (x.rows() x xd.rows())
   * @param x     Function-value observation locations
   * @param xd    Derivative observation locations
   * @param dim   Input dimension for the derivative
   */
  virtual void computeCovarianceFD(RealEigenMatrix & K_fd,
                                   const RealEigenMatrix & x,
                                   const RealEigenMatrix & xd,
                                   unsigned int dim) const;

  /**
   * Compute the cross-covariance between derivatives and function values:
   *   K_df[i,j] = Cov[df(xd_i)/dx_{dim}, f(xp_j)] = dK(xd_i, xp_j)/dx_{d,dim}
   * For SE kernel: K_df[i,j] = K(xd_i,xp_j) * (xp_{j,dim} - xd_{i,dim}) / ell_dim^2
   * Note: K_df = K_fd^T when xd and xp are swapped.
   * Default implementation calls mooseError.
   * @param K_df  Output matrix of size (xd.rows() x xp.rows())
   * @param xd    Derivative observation locations
   * @param xp    Function-value locations (e.g. test points)
   * @param dim   Input dimension for the derivative
   */
  virtual void computeCovarianceDf(RealEigenMatrix & K_df,
                                   const RealEigenMatrix & xd,
                                   const RealEigenMatrix & xp,
                                   unsigned int dim) const;

  /**
   * Compute the covariance between two sets of derivative observations:
   *   K_dd[i,j] = Cov[df(xd_i)/dx_{dim_i}, df(xdp_j)/dx'_{dim_j}]
   *             = d^2 K(xd_i, xdp_j) / (dx_{dim_i} dx'_{dim_j})
   * For SE kernel when dim_i == dim_j:
   *   = K(xd_i,xdp_j) * [1/ell_k^2 - (xd_{i,k}-xdp_{j,k})^2/ell_k^4]
   * For SE kernel when dim_i != dim_j:
   *   = K(xd_i,xdp_j) * [-(xd_{i,ki}-xdp_{j,ki})*(xd_{i,kj}-xdp_{j,kj})/(ell_ki^2*ell_kj^2)]
   * Default implementation calls mooseError.
   * @param K_dd   Output matrix of size (xd.rows() x xdp.rows())
   * @param xd     First set of derivative observation locations
   * @param xdp    Second set of derivative observation locations
   * @param dim_i  Derivative dimension for the first set
   * @param dim_j  Derivative dimension for the second set
   */
  virtual void computeCovarianceDD(RealEigenMatrix & K_dd,
                                   const RealEigenMatrix & xd,
                                   const RealEigenMatrix & xdp,
                                   unsigned int dim_i,
                                   unsigned int dim_j) const;

  /**
   * Compute dK_cross/dhp: the derivative of the cross-covariance K(x, xc) with
   * respect to a hyperparameter. Used in the penalty constraint gradient.
   * For kernels that do not implement this, the default returns a zero matrix
   * (penalty loss is computed but no gradient contribution is added).
   * @param dKdhp      Output matrix of size (x.rows() x xc.rows())
   * @param x          Training point locations (batch)
   * @param xc         Constraint point location (1 x d)
   * @param hp_name    Hyperparameter name (prefixed with covariance object name)
   * @param ind        Index within vector hyperparameter (0 for scalars)
   */
  virtual void computedKdhyper_cross(RealEigenMatrix & dKdhp,
                                     const RealEigenMatrix & x,
                                     const RealEigenMatrix & xc,
                                     const std::string & hp_name,
                                     unsigned int ind) const;

  /// Check if a given parameter is tunable
  /// @param The name of the hyperparameter
  virtual bool isTunable(const std::string & name) const;

  /// Return the number of outputs assumed for this covariance function
  unsigned int numOutputs() const { return _num_outputs; }

  /// Get the map of scalar parameters
  std::unordered_map<std::string, Real> & hyperParamMapReal() { return _hp_map_real; }

  /// Get the map of vector parameters
  std::unordered_map<std::string, std::vector<Real>> & hyperParamMapVectorReal()
  {
    return _hp_map_vector_real;
  }

protected:
  /// Register a scalar hyperparameter to this covariance function
  /// @param name The name of the parameter
  /// @param value The initial value of the parameter
  /// @param is_tunable If the parameter is tunable during optimization
  const Real &
  addRealHyperParameter(const std::string & name, const Real value, const bool is_tunable);

  /// Register a vector hyperparameter to this covariance function
  /// @param name The name of the parameter
  /// @param value The initial value of the parameter
  /// @param is_tunable If the parameter is tunable during optimization
  const std::vector<Real> & addVectorRealHyperParameter(const std::string & name,
                                                        const std::vector<Real> value,
                                                        const bool is_tunable);

  /// Map of real-valued hyperparameters
  std::unordered_map<std::string, Real> _hp_map_real;

  /// Map of vector-valued hyperparameters
  std::unordered_map<std::string, std::vector<Real>> _hp_map_vector_real;

  /// list of tunable hyper-parameters
  std::unordered_set<std::string> _tunable_hp;

  /// The number of outputs this covariance function is used to describe
  const unsigned int _num_outputs;

  /// The names of the dependent covariance functions
  const std::vector<UserObjectName> _dependent_covariance_names;

  /// The types of the dependent covariance functions
  std::vector<std::string> _dependent_covariance_types;

  /// Vector of pointers to the dependent covariance functions
  std::vector<CovarianceFunctionBase *> _covariance_functions;
};
