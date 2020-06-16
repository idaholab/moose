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

  ///
  const DenseMatrix<Real> & _parameter_mat;

  /// An _n_sample vector containg results of traing runs
  const DenseMatrix<Real> & _training_results;

  /// An _n_sample vector containg results of traing runs
  const DenseMatrix<Real> & _training_mean;

  /// An _n_sample vector containg results of traing runs
  const DenseMatrix<Real> & _training_variance;

  /// An _n_sample by _n_sample covariance matrix constructed from the selected kernel function
  const DenseMatrix<Real> & _K;

  /// A solve of Ax=b via Cholesky.
  const DenseMatrix<Real> & _K_results_solve;

  ///
  const DenseMatrix<Real> & _K_cho_decomp;

  ///
  const std::unique_ptr<CovarianceFunction::CovarianceKernel> & _covar_function;

};
