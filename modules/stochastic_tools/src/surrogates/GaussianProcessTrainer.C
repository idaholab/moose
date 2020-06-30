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
  params.addClassDescription(
      "Provides data preperation and training for a Gaussian Process surrogate model.");
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
    _param_standardizer(declareModelData<StochasticTools::Standardizer>("_param_standardizer")),
    _training_data(declareModelData<RealEigenMatrix>("_training_data")),
    _data_standardizer(declareModelData<StochasticTools::Standardizer>("_data_standardizer")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _covar_function(
        declareModelData<std::unique_ptr<CovarianceFunction::CovarianceKernel>>("_covar_function")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data"))
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

  _covar_function = CovarianceFunction::makeCovarianceKernel(_kernel_type, this);

  unsigned int num_samples = _values_ptr->size();

  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Load training parameters into Eigen::Matrix for easier liner algebra manipulation
  // Load training data into Eigen::Vector for easier liner algebra manipulation
  mooseAssert(_sampler->getLocalSamples().m() == num_samples,
              "Number of sampler rows not equal to number of results in selected VPP.");

  // Consider the possibility of a very large matrix load.
  _training_params.resize(_sampler->getLocalSamples().m(), _sampler->getLocalSamples().n());
  _training_data.resize(num_samples, 1);
  int row_num = 0;
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p, ++row_num)
  {
    // Loading parameters from sampler
    std::vector<Real> data = _sampler->getNextLocalRow();
    for (unsigned int d = 0; d < data.size(); ++d)
      _training_params(row_num, d) = data[d];

    // Loading result data from VPP
    _training_data(row_num, 0) = (*_values_ptr)[row_num - offset];
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
  {
    _param_standardizer.computeSet(_training_params);
    _training_params = _param_standardizer.getStandardized(_training_params);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);

  // Standardize (center and scale) training data
  if (_standardize_data)
  {
    _data_standardizer.computeSet(_training_data);
    _training_data = _data_standardizer.getStandardized(_training_data);
  }
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _param_standardizer.set(0, 1, _n_params);
}

void
GaussianProcessTrainer::execute()
{
  _K = _covar_function->compute_K(_training_params, _training_params, true);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(_training_data);
}

void
GaussianProcessTrainer::finalize()
{
}
