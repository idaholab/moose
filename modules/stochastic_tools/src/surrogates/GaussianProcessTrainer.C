//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessTrainer.h"
#include "Sampler.h"
#include "CartesianProduct.h"

registerMooseObject("StochasticToolsApp", GaussianProcessTrainer);

InputParameters
GaussianProcessTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params += CovarianceInterface::validParams();
  params.addClassDescription(
      "Provides data preperation and training for a Gaussian Process surrogate model.");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");

  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    CovarianceInterface(parameters),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _param_standardizer(declareModelData<StochasticTools::Standardizer>("_param_standardizer")),
    _training_data(),
    _data_standardizer(declareModelData<StochasticTools::Standardizer>("_data_standardizer")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _covar_type(declareModelData<std::string>("_covar_type")),
    _hyperparam_map(declareModelData<std::unordered_map<std::string, Real>>("_hyperparam_map")),
    _hyperparam_vec_map(declareModelData<std::unordered_map<std::string, std::vector<Real>>>(
        "_hyperparam_vec_map")),
    _covariance_function(
        getCovarianceFunctionByName(getParam<UserObjectName>("covariance_function")))

{
}

void
GaussianProcessTrainer::initialSetup()
{

  // Results VPP
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr = &getVectorPostprocessorValue(
      "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

  // Sampler
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _n_params = _sampler->getNumberOfCols();

  // Check if sampler dimension matches number of distributions
  std::vector<DistributionName> dname = getParam<std::vector<DistributionName>>("distributions");
  if (dname.size() != _n_params)
    mooseError("Sampler number of columns does not match number of inputted distributions.");
}

void
GaussianProcessTrainer::initialize()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();

  if (num_rows != _values_ptr->size())
    paramError("results_vpp",
               "The number of elements in '",
               getParam<VectorPostprocessorName>("results_vpp"),
               "/",
               getParam<std::string>("results_vector"),
               "' is not equal to the number of samples in '",
               getParam<SamplerName>("sampler"),
               "'!");

  _covar_type = _covariance_function->type();

  mooseAssert(_sampler->getNumberOfRows() == _values_ptr->size(),
              "Number of sampler rows not equal to number of results in selected VPP.");
}

void
GaussianProcessTrainer::execute()
{
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Consider the possibility of a very large matrix load.
  _training_params.setZero(_sampler->getNumberOfRows(), _sampler->getNumberOfCols());
  _training_data.setZero(_sampler->getNumberOfRows(), 1);
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    // Loading parameters from sampler
    std::vector<Real> data = _sampler->getNextLocalRow();
    for (unsigned int d = 0; d < data.size(); ++d)
      _training_params(p, d) = data[d];

    // Loading result data from VPP
    _training_data(p, 0) = (*_values_ptr)[p - offset];
  }
}

void

GaussianProcessTrainer::finalize()
{
  for (unsigned int ii = 0; ii < _training_params.rows(); ++ii)
  {
    for (unsigned int jj = 0; jj < _training_params.cols(); ++jj)
      gatherSum(_training_params(ii, jj));
    gatherSum(_training_data(ii, 0));
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
  {
    _param_standardizer.computeSet(_training_params);
    _param_standardizer.getStandardized(_training_params);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);

  // Standardize (center and scale) training data
  if (_standardize_data)
  {
    _data_standardizer.computeSet(_training_data);
    _data_standardizer.getStandardized(_training_data);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);

  _covariance_function->buildHyperParamMap(_hyperparam_map, _hyperparam_vec_map);
  _K.resize(_training_params.rows(), _training_params.rows());
  _covariance_function->computeCovarianceMatrix(_K, _training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);
}
