//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "GaussianProcessTorched.h"

#include "LibtorchUtils.h"

class GaussianProcessSurrogateTorched : public SurrogateModel, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcessSurrogateTorched(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const;
  virtual void evaluate(const std::vector<Real> & x, std::vector<Real> & y) const;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const;
  virtual void
  evaluate(const std::vector<Real> & x, std::vector<Real> & y, std::vector<Real> & std) const;

  /**
   * This function is called by LoadCovarianceDataAction when the surrogate is
   * loading training data from a file. The action must recreate the covariance
   * object before this surrogate can set the correct pointer.
   */
  virtual void setupCovariance(UserObjectName _covar_name);

  StochasticToolsTorched::GaussianProcessTorched & gp() { return _gp; }
  const StochasticToolsTorched::GaussianProcessTorched & getGP() const { return _gp; }

private:
  StochasticToolsTorched::GaussianProcessTorched & _gp;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;
};
