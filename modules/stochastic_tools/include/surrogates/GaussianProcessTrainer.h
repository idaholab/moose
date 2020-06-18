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

  static RealEigenMatrix cholesky_back_substitute(const RealEigenMatrix & A, const RealEigenMatrix & b);

protected:

private:
  // TODO: Move as much of these to constructor initialization

  /// Sampler from which the parameters were perturbed
  Sampler * _sampler;

  /// Vector postprocessor of the results from perturbing the model with _sampler
  const VectorPostprocessorValue * _values_ptr = nullptr;

  /// True when _sampler data is distributed
  bool _values_distributed;

  // The following items are stored using declareModelData for use as a trained model.

  /// Total number of parameters/dimensions
  unsigned int _n_params;

  // /// Vector of length factors. Main hyper-parameters for squared exponential kernel
  // std::vector<Real> & _length_factor;

  //
  const MooseEnum & _kernel_type;

  ///Paramaters (x) used for training
  RealEigenMatrix & _training_params;
  RealEigenMatrix & _training_params_mean;
  RealEigenMatrix & _training_params_var;

  /// Data (y) used for training
  RealEigenMatrix & _training_data;
  Real & _training_data_mean;
  Real & _training_data_var;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  RealEigenMatrix & _K_results_solve;

  ///
  std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;

  ///
  Eigen::LLT<RealEigenMatrix>  _K_cho_decomp;

};
