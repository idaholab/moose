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
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  MooseEnum kernels = CovarianceFunction::makeCovarianceKernelEnum();
  params.addRequiredParam<MooseEnum>(
      "kernel_function", kernels, "The kernel (covariance function) to use.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length Factor to use for Covariance Kernel");
  params.addParam<Real>(
      "signal_variance", 1, "Signal Variance (sigma_f^2) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0, "Noise Variance (sigma_n^2) to use for kernel calculation.");
  params.addParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");
  params.addParam<unsigned int>("p", "Integer p to use for Matern Hald Integer Covariance Kernel");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");

  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _kernel_type(getParam<MooseEnum>("kernel_function")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _training_params_mean(declareModelData<RealEigenVector>("_training_params_mean")),
    _training_params_var(declareModelData<RealEigenVector>("_training_params_var")),
    _training_data(declareModelData<RealEigenMatrix>("_training_data")),
    _training_data_mean(declareModelData<RealEigenVector>("_training_data_mean")),
    _training_data_var(declareModelData<RealEigenVector>("_training_data_var")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _covar_function(
        declareModelData<std::unique_ptr<CovarianceFunction::CovarianceKernel>>("_covar_function")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data"))
{
}

void
GaussianProcessTrainer::initialize()
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

  _covar_function = CovarianceFunction::makeCovarianceKernel(_kernel_type, this);
}

void
GaussianProcessTrainer::execute()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  unsigned int _num_samples = _values_ptr->size();

  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // May load a very large matrix, which could be bad.
  DenseMatrix<Real> params = _sampler->getLocalSamples();
  _training_params.resize(params.m(), params.n());
  for (unsigned int ii = 0; ii < _training_params.rows(); ii++)
  {
    for (unsigned int jj = 0; jj < _training_params.cols(); jj++)
    {
      _training_params(ii, jj) = params(ii, jj);
    }
  }

  if (_standardize_params)
  {
    // comptue mean
    _training_params_mean = _training_params.colwise().mean();
    // center params
    _training_params = _training_params.rowwise() - _training_params_mean.transpose();
    // compute variance
    _training_params_var = _training_params.colwise().squaredNorm() / _num_samples;
    // scale params using std
    _training_params =
        _training_params.array().rowwise() / _training_params_var.transpose().array().sqrt();
  }
  else
  {
    // if not standardizing data set mean=0, std=1 for use in surrogate
    _training_params_mean = RealEigenVector::Zero(_n_params);
    _training_params_var = RealEigenVector::Ones(_n_params);
  }

  // Load training data into Eigen::Vector for easy manipulation
  _training_data.resize(_num_samples, 1);
  for (unsigned int ii = 0; ii < _num_samples; ii++)
  {
    _training_data(ii, 0) = (*_values_ptr)[ii - offset];
  }

  if (_standardize_data)
  {
    // comptue mean
    _training_data_mean = _training_data.colwise().mean();
    // center data
    _training_data = _training_data.rowwise() - _training_data_mean.transpose();
    // compute variance
    _training_data_var = _training_data.colwise().squaredNorm() / _num_samples;
    // scale data using std
    _training_data =
        _training_data.array().rowwise() / _training_data_var.transpose().array().sqrt();
  }
  else
  {
    // if not standardizing data set mean=0, std=1 for use in surrogate
    _training_data_mean = RealEigenVector::Zero(1);
    _training_data_var = RealEigenVector::Ones(1);
  }

  _K = _covar_function->compute_K(_training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);
}

void
GaussianProcessTrainer::finalize()
{
}
