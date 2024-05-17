//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessSurrogateGeneral.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", GaussianProcessSurrogateGeneral);

InputParameters
GaussianProcessSurrogateGeneral::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

GaussianProcessSurrogateGeneral::GaussianProcessSurrogateGeneral(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _gp(declareModelData<StochasticTools::GaussianProcessGeneral>("_gp")),
    _training_params(getModelData<RealEigenMatrix>("_training_params")),
    _n_dims(getModelData<unsigned int>("_n_dims")),
    _n_outputs(getModelData<unsigned int>("_n_outputs"))
{
}

void
GaussianProcessSurrogateGeneral::setupCovariance(UserObjectName covar_name)
{
  if (_gp.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _gp.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
}

Real
GaussianProcessSurrogateGeneral::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
GaussianProcessSurrogateGeneral::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  std::vector<Real> y;
  std::vector<Real> std;
  this->evaluate(x, y, std);
  std_dev = std[0];
  return y[0];
}

void
GaussianProcessSurrogateGeneral::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  std::vector<Real> std_dummy;
  this->evaluate(x, y, std_dummy);
}

void
GaussianProcessSurrogateGeneral::evaluate(const std::vector<Real> & x,
                                          std::vector<Real> & y,
                                          std::vector<Real> & std) const
{

  mooseAssert(x.size() == _n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  y = std::vector<Real>(_n_outputs, 0.0);
  std = std::vector<Real>(_n_outputs, 0.0);

  RealEigenMatrix test_points(1, _n_dims);
  for (unsigned int ii = 0; ii < _n_dims; ++ii)
    test_points(0, ii) = x[ii];

  _gp.getParamStandardizer().getStandardized(test_points);

  RealEigenMatrix K_train_test(_training_params.rows() * _n_outputs, _n_outputs);
  _gp.getCovarFunction().computeCovarianceMatrix(
      K_train_test, _training_params, test_points, false);
  RealEigenMatrix K_test(_n_outputs, _n_outputs);
  _gp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // Compute the predicted mean value (centered)
  RealEigenMatrix pred_value = (K_train_test.transpose() * _gp.getKResultsSolve()).transpose();
  // De-center/scale the value and store for return
  _gp.getDataStandardizer().getDestandardized(pred_value);

  RealEigenMatrix pred_var =
      K_test - (K_train_test.transpose() * _gp.getKCholeskyDecomp().solve(K_train_test));

  // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _gp.getDataStandardizer().getDescaled(std_dev_mat);

  for (const auto output_i : make_range(_n_outputs))
  {
    y[output_i] = pred_value(0, output_i);
    std[output_i] = std_dev_mat(output_i, output_i);
  }
}
