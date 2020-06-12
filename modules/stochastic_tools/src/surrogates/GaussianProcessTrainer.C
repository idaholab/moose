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
  params.addRequiredParam<std::vector<Real> >("length_factor", "Length Factor to use for Covariance Kernel");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");

  return params;
}

GaussianProcessTrainer::GaussianProcessTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _length_factor(declareModelData<std::vector<Real> >("_length_factor")),
    _parameter_mat(declareModelData<DenseMatrix<Real> >("_parameter_mat")),
    _covariance_mat(declareModelData<DenseMatrix<Real> >("_covariance_mat")),
    _training_results(declareModelData<DenseMatrix<Real> >("_training_results")),
    _training_mean(declareModelData<DenseMatrix<Real> >("_training_mean")),
    _training_variance(declareModelData<DenseMatrix<Real> >("_training_variance")),
    _covariance_results_solve(declareModelData<DenseMatrix<Real> >("_covariance_results_solve")),
    _covariance_mat_cho_decomp(declareModelData<DenseMatrix<Real> >("_covariance_mat_cho_decomp"))
{
}

void
GaussianProcessTrainer::initialize()
{
  _length_factor=getParam<std::vector<Real>>("length_factor");
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

DenseMatrix<Real>
GaussianProcessTrainer::cholesky_back_substitute(const DenseMatrix<Real> & A, const DenseMatrix<Real> & b)
{
  unsigned int n_cols=A.n();
  //verify A.n == A.m == b.n
  unsigned int n_solves=b.n();
  //std::cout << n_cols << A.m() << b.n() << n_solves <<'\n';
  DenseMatrix<Real> x(n_cols,n_solves);


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
  _parameter_mat = _sampler->getLocalSamples();


  //Get training data
  _training_results.resize(_num_samples,1);
  _training_mean.resize(1,1);
  _training_variance.resize(1,1);
  for (unsigned int ii=0; ii<_num_samples; ii++){
    _training_results(ii,0) =   (*_values_ptr)[ii - offset];
    _training_mean(0,0) += (*_values_ptr)[ii - offset] / _num_samples;
  }
  for (unsigned int ii=0; ii<_num_samples; ii++){
    _training_variance(0,0) +=  std::pow((_training_results(ii,0)-_training_mean(0,0)),2) / _num_samples;
  }

  //populate covariance mat
  _covariance_mat.resize(_num_samples, _num_samples);
  for (unsigned int ii=0; ii<_num_samples; ii++){
    for (unsigned int jj=0; jj<_num_samples; jj++){
        //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
        Real cov =0;
        for (unsigned int kk=0; kk<_n_params; kk++){
          //Compute distance per parameter
          cov += std::pow(( _parameter_mat(ii,kk)-  _parameter_mat(jj,kk) ),2) / (std::pow(_length_factor.at(kk),2));
        }
        cov = _training_variance(0,0) * std::exp(-cov) / 2.0;
        _covariance_mat(ii,jj) = cov;
        _covariance_mat(jj,ii) = cov;
      }
  }



  //This is annoying, find better way to go between DenseMatrix and DenseVector
  DenseVector<Real> _training_results_vec(_num_samples);
  DenseMatrix<Real> _training_results_centered(_training_results);
  for (unsigned int ii=0; ii<_num_samples; ii++){
    _training_results_vec(ii)=_training_results(ii,0) - _training_mean(0,0);
    _training_results_centered(ii,0) = _training_results(ii,0) - _training_mean(0,0);
  }
  //Perform Initial Cholesky Solve. We are more interested in the decomposed matrix.
  _covariance_mat_cho_decomp = _covariance_mat;
    DenseVector<Real> cho_solution_vec;
  _covariance_mat_cho_decomp.cholesky_solve(_training_results_vec,cho_solution_vec);

  // std::cout << "After Cho Solve" << '\n';
  //  _covariance_mat.print();
  //  std::cout << "Training results"<< '\n';
  //  _training_results.print();
  //  std::cout << "Cho Solution LibMesh Vec" << '\n';
  //  cho_solution_vec.print(std::cout);
  //  std::cout << "Cho Solution Static Function" << '\n';
   _covariance_results_solve = cholesky_back_substitute(_covariance_mat_cho_decomp, _training_results_centered);
   //_covariance_results_solve.print();

   _parameter_mat.print();
}

void
GaussianProcessTrainer::finalize()
{
}
