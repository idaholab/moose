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
#include "Standardizer.h"
#include <Eigen/Dense>
#include "CovarianceInterface.h"
#include "TwoLayerGaussianProcess.h"

class TwoLayerGaussianProcessSurrogate : public SurrogateModel, public CovarianceInterface
{
public:
  static InputParameters validParams();
  TwoLayerGaussianProcessSurrogate(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const;
  virtual void evaluate(const std::vector<Real> & x, std::vector<Real> & y) const;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const;
  virtual void
  evaluate(const std::vector<Real> & x, std::vector<Real> & y, std::vector<Real> & std) const;

  // void predict();

  /**
   * This function is called by LoadCovarianceDataAction when the surrogate is
   * loading training data from a file. The action must recreate the covariance
   * object before this surrogate can set the correct pointer.
   */
  virtual void setupCovariance(UserObjectName _covar_name);

  StochasticTools::TwoLayerGaussianProcess & tgp() { return _tgp; }
  const StochasticTools::TwoLayerGaussianProcess & getTGP() const { return _tgp; }

  // struct KrigResult {
  //   RealEigenMatrix mean;
  //   RealEigenMatrix sigma;
  // };

  // void squared_exponential_covariance(const RealEigenMatrix &x1, 
  //                 const RealEigenMatrix &x2, 
  //                 Real tau2, 
  //                 const RealEigenMatrix &theta, 
  //                 Real g, 
  //                 RealEigenMatrix &k);

  // void krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
  //                                  const RealEigenMatrix & theta, Real g, Real tau2, bool cal_sigma,
  //                                  const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new, KrigResult & result);

  // void predict(const RealEigenMatrix & x_new);

private:
  StochasticTools::TwoLayerGaussianProcess & _tgp;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;


};
