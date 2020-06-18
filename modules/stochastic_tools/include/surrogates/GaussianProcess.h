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
  //virtual void initialize();
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real* std) const;

  //static DenseMatrix<Real> cholesky_back_substitute(const DenseMatrix<Real> & A, const DenseMatrix<Real> & b);

protected:
  /// Array containing sample points and the results
  const std::vector<std::vector<Real>> & _sample_points;

private:

  //const std::vector<Real> & _length_factor;

  ///Paramaters (x) used for training
  const RealEigenMatrix & _training_params;
  const RealEigenMatrix & _training_params_mean;
  const RealEigenMatrix & _training_params_var;

  /// Data (y) used for training
  const RealEigenMatrix & _training_data;
  const Real & _training_data_mean;
  const Real & _training_data_var;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  const RealEigenMatrix & _K;

  /// A solve of Ax=b via Cholesky.
  const RealEigenMatrix & _K_results_solve;

  ///
  const std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;

};
