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
#include <Eigen/Dense>

class GaussianProcess : public SurrogateModel
{
public:
  static InputParameters validParams();
  GaussianProcess(const InputParameters & parameters);
  // virtual void initialize();
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const;

protected:
  /// Array containing sample points and the results
  const std::vector<std::vector<Real>> & _sample_points;

private:
  ///Paramaters (x) used for training, along with statistics
  const RealEigenMatrix & _training_params;
  const RealEigenVector & _training_params_mean;
  const RealEigenVector & _training_params_var;

  /// Data (y) used for training, along with statistics
  const RealEigenMatrix & _training_data;
  const RealEigenVector & _training_data_mean;
  const RealEigenVector & _training_data_var;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  const RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  const RealEigenMatrix & _K_results_solve;

  ///Pointer to covariance function used for K matrix
  const std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;
};
