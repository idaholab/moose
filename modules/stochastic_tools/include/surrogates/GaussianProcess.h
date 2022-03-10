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
#include "CovarianceInterface.h"
#include "GaussianProcessUtils.h"

class GaussianProcess : public SurrogateModel, public CovarianceInterface
{
public:
  static InputParameters validParams();
  GaussianProcess(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual Real evaluate(const std::vector<Real> & x, Real & std) const override;

  StochasticTools::GaussianProcessUtils & gpUtils() { return _gp_utils; }
  const StochasticTools::GaussianProcessUtils & getGPUtils() const { return _gp_utils; }

private:
  StochasticTools::GaussianProcessUtils & _gp_utils;

  /// Paramaters (x) used for training
  const RealEigenMatrix & _training_params;
};
