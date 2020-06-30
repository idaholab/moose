//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateModel.h"
#include "GaussianProcessTrainer.h"
#include "Standardizer.h"
#include <Eigen/Dense>

class GaussianProcess : public SurrogateModel
{
public:
  static InputParameters validParams();
  GaussianProcess(const InputParameters & parameters);
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const;

private:
  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;

  /// Standardizer for use with params (x)
  const StochasticTools::Standardizer & _param_standardizer;

  /// Data (y) used for training
  const RealEigenMatrix & _training_data;

  /// Standardizer for use with data (y)
  const StochasticTools::Standardizer & _data_standardizer;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  const RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  const RealEigenMatrix & _K_results_solve;

  /// Pointer to covariance function used for K matrix
  const std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;
};
