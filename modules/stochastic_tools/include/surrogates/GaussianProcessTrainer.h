//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateTrainer.h"
#include "CovarianceFunction.h"
#include "Standardizer.h"
#include <Eigen/Dense>

#include "Distribution.h"

class GaussianProcessTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();
  GaussianProcessTrainer(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
private:
  /// Sampler from which the parameters were perturbed
  Sampler * _sampler;

  /// Vector postprocessor of the results from perturbing the model with _sampler
  const VectorPostprocessorValue * _values_ptr = nullptr;

  /// True when _sampler data is distributed
  bool _values_distributed;

  /// Total number of parameters/dimensions
  unsigned int _n_params;

  // Type of kernel function to be used. See CovarianceFunction for options
  const MooseEnum & _kernel_type;

  /// Paramaters (x) used for training, along with statistics
  RealEigenMatrix & _training_params;

  /// Standardizer for use with params (x)
  StochasticTools::Standardizer & _param_standardizer;

  /// Data (y) used for training
  RealEigenMatrix & _training_data;

  /// Standardizer for use with data (y)
  StochasticTools::Standardizer & _data_standardizer;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  RealEigenMatrix & _K_results_solve;

  /// Pointer to covariance function used for K matrix
  std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;

  /// Cholesky decomposition Eigen object
  Eigen::LLT<RealEigenMatrix> _K_cho_decomp;

  /// Switch for training param (x) standardization
  bool _standardize_params;

  /// Switch for training data(y) standardization
  bool _standardize_data;
};
