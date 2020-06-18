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
      "kernel_function",
      kernels,
      "The kernel (covariance function) to use.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");
  params.addRequiredParam<std::vector<Real> >("length_factor", "Length Factor to use for Covariance Kernel");
  params.addParam<Real>("gamma", "Gamma to use for Exponential Covariance Kernel");

  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _kernel_type(getParam<MooseEnum>("kernel_function")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _training_params_mean(declareModelData<RealEigenMatrix>("_training_params_mean")),
    _training_params_var(declareModelData<RealEigenMatrix>("_training_params_var")),
    _training_data(declareModelData<RealEigenMatrix>("_training_data")),
    _training_data_mean(declareModelData<Real>("_training_data_mean")),
    _training_data_var(declareModelData<Real>("_training_data_var")),
    _K(declareModelData<RealEigenMatrix>("_K")),
    _K_results_solve(declareModelData<RealEigenMatrix>("_K_results_solve")),
    _covar_function(declareModelData<std::unique_ptr<CovarianceFunction::CovarianceKernel> >("_covar_function"))
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

RealEigenMatrix
GaussianProcessTrainer::cholesky_back_substitute(const RealEigenMatrix & A, const RealEigenMatrix & b)
{
  unsigned int n_cols=A.cols();
  //verify A.n == A.m == b.n
  unsigned int n_solves=b.cols();
  //std::cout << n_cols << A.m() << b.n() << n_solves <<'\n';
  RealEigenMatrix x(n_cols,n_solves);


  //loosly modified
  for (unsigned int s=0; s<n_solves; ++s){
    // Solve for Ly=b
    //std::cout << "s " << s << '\n';
    for (unsigned int i=0; i<n_cols; ++i)
    {
      //std::cout << "i " << i << '\n';
      Real temp = b(i,s);
      for (unsigned int k=0; k<i; ++k){
        temp -= A(i,k)*x(k,s);
      }
      x(i,s) = temp / A(i,i);
    }

    // Solve for L^T x = y
    for (unsigned int i=0; i<n_cols; ++i)
    {
      //std::cout << "i " << i << '\n';
      const unsigned int ib = (n_cols-1)-i;
      for (unsigned int k=(ib+1); k<n_cols; ++k){
        x(ib,s) -= A(k,ib) * x(k,s);
      }
      x(ib,s) /= A(ib,ib);
    }
  }
  return x;
}

void
GaussianProcessTrainer::execute()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  // Initialize covariance matrix
  unsigned int _num_samples = _values_ptr->size();

  //std::cout << "_covariance_mat " << '\n' << _covariance_mat << '\n' << '\n';
  //

  //TODO:figure this out
  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  //load this into matrix for the time being, optimize later with proper Calls.
  //Arbitary access is helpfor for initial dev.
  //May load a very large matrix, which could be bad.
  DenseMatrix<Real> params = _sampler->getLocalSamples();
  _training_params.resize(params.m(),params.n());
  for (unsigned int ii=0; ii<_training_params.rows(); ii++){
    for (unsigned int jj=0; jj<_training_params.cols(); jj++){
      _training_params(ii,jj)=params(ii,jj);
    }
  }

  std::cout << _training_params << '\n';
  _training_params_mean=_training_params.colwise().mean();
  std::cout << "mean" <<'\n' << _training_params.colwise().mean() << '\n';
  //_training_data_var = (_training_params.array().colwise() - _training_params_mean.array()).squaredNorm() / _num_samples;
  //_training_data_var = _training_params.array().colwise() - _training_params_mean.array()
  Eigen::Map<Eigen::VectorXd> tmp(_training_params_mean.data(),_training_params_mean.size());
  std::cout << "var" <<'\n' << (_training_params.rowwise() - tmp.transpose()).colwise().squaredNorm() / _num_samples << '\n';

  //Get training data
  _training_data.resize(_num_samples,1);
  //_training_data_mean.resize(1,1);
  //_training_data_var.resize(1,1);
  for (unsigned int ii=0; ii<_num_samples; ii++){
    _training_data(ii,0) =   (*_values_ptr)[ii - offset];
  }
  //compute mean
  _training_data_mean = _training_data.mean();
  RealEigenMatrix training_resutls_centered = _training_data.array()-_training_data_mean;


  // std::cout << "Results" << '\n';
  // std::cout << _training_data << '\n';
  // std::cout << "Mean" << '\n';
  // std::cout << _training_data_mean << '\n';
  // std::cout << "Centered" << '\n';
  // std::cout << training_resutls_centered << '\n';

  _training_data_var= training_resutls_centered.squaredNorm() / _num_samples;

  _covar_function->set_signal_variance(_training_data_var);
  _K = _covar_function->compute_matrix(_training_params,_training_params);
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve( training_resutls_centered );
  //std::cout << _K_results_solve  << '\n';
  //std::cout << _K << '\n';


}

void
GaussianProcessTrainer::finalize()
{
}
