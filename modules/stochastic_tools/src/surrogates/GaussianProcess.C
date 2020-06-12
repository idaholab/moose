//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcess.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", GaussianProcess);

InputParameters
GaussianProcess::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  //params.addRequiredParam<std::vector<Real> >("length_factor", "Length Factor to use for Covariance Kernel");
  return params;
}

GaussianProcess::GaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
  _sample_points(getModelData<std::vector<std::vector<Real>>>("_sample_points")),
  //_length_factor(getParam<std::vector<Real>>("length_factor")),
  _length_factor(getModelData<std::vector<Real> >("_length_factor")),
  _parameter_mat(getModelData<DenseMatrix<Real> >("_parameter_mat")),
  _training_results(getModelData<DenseMatrix<Real> >("_training_results")),
  _training_mean(getModelData<DenseMatrix<Real> >("_training_mean")),
  _training_variance(getModelData<DenseMatrix<Real> >("_training_variance")),
  _covariance_mat(getModelData<DenseMatrix<Real> >("_covariance_mat")),
  _covariance_results_solve(getModelData<DenseMatrix<Real> >("_covariance_results_solve")),
  _covariance_mat_cho_decomp(getModelData<DenseMatrix<Real> >("_covariance_mat_cho_decomp"))
{
   std::cout << "In Constructor" << '\n';
     _parameter_mat.print();
}

// void
// GaussianProcess::initialize(){
//   //Get some basic stats related to total number of paramaters used in the model
//   //as well as total samples used for training
//   _n_params = _parameter_mat.n();
//   _num_samples = _parameter_mat.m();
//   std::cout << "In initialize!" << '\n';
//   _parameter_mat.print();
//   _num_tests=1;
//
//   _covariance_mat.resize(_num_samples, _num_samples);
//
//   //populate covariance mat
//   for (unsigned int ii=0; ii<_num_samples; ii++){
//     for (unsigned int jj=0; jj<_num_samples; jj++){
//         //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
//         Real cov =0;
//         for (unsigned int kk=0; kk<_n_params; kk++){
//           //Compute distance per parameter
//           cov += std::pow(( _parameter_mat(ii,kk)-  _parameter_mat(jj,kk) ),2) / (std::pow(_length_factor.at(kk),2));
//         }
//         cov = _training_variance(0,0) * std::exp(-cov) / 2.0;
//         _covariance_mat(ii,jj) = cov;
//         _covariance_mat(jj,ii) = cov;
//       }
//   }
//
//   //This is annoying, find better way to go between DenseMatrix and DenseVector
//   DenseVector<Real> training_results_vec(_num_samples);
//   DenseMatrix<Real> training_results_centered(_training_results);
//   for (unsigned int ii=0; ii<_num_samples; ii++){
//     training_results_vec(ii)=_training_results(ii,0) - _training_mean(0,0);
//     training_results_centered(ii,0) = _training_results(ii,0) - _training_mean(0,0);
//   }
//   //Perform Initial Cholesky Solve. We are more interested in the decomposed matrix.
//   _covariance_mat_cho_decomp = _covariance_mat;
//   DenseVector<Real> cho_solution_vec;
//   _covariance_mat_cho_decomp.cholesky_solve(training_results_vec,cho_solution_vec);
//   _covariance_results_solve = cholesky_back_substitute(_covariance_mat_cho_decomp, training_results_centered);
// }

Real
GaussianProcess::evaluate(const std::vector<Real> & x) const
{
  //overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy=0;
  return this->evaluate(x, & dummy);
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x, Real* std_dev) const
{
  unsigned int _n_params = _parameter_mat.n();
  unsigned int _num_samples = _parameter_mat.m();
  unsigned int _num_tests=1;

   //BEGIN UNIQUE SOLVE
   std::cout << "In evaluate!" << '\n';
   _parameter_mat.print();

  DenseMatrix<Real> K_train_test(_num_samples,_num_tests);
  DenseMatrix<Real> K_test(_num_tests,_num_tests);

  //Load Kernel matrix section consisting of training--testing covariance values
  for (unsigned int ii=0; ii<_num_samples; ii++){
    for (unsigned int jj=0; jj<_num_tests; jj++){
      Real cov =0;
      for (unsigned int kk=0; kk<_n_params; kk++){
        cov +=  std::pow(( _parameter_mat(ii,kk)-  x.at(kk)),2) / (std::pow(_length_factor.at(kk),2));
      }
      cov = _training_variance(0,0) * std::exp(-cov) / 2.0;
      K_train_test(ii,jj) = cov;
      }
  }

  //Load Kernel matrix section for testing--testing covariance values
  //When num_tests==1 this sinple produces a 1x1 matrix (a scalar), but useful to have this in
  //a matrix object for math
  for (unsigned int ii=0; ii<_num_tests; ii++){
    for (unsigned int jj=0; jj<_num_tests; jj++){
      Real cov =0;
      for (unsigned int kk=0; kk<_n_params; kk++){
        cov +=  std::pow(( x.at(kk)-  x.at(kk)),2) / (std::pow(_length_factor.at(kk),2));
      }
      cov = _training_variance(0,0) * std::exp(-cov) / 2.0;
      K_test(ii,jj) = cov;
      }
  }

  //Compute the predicted mean value (centered)
  DenseMatrix<Real> test_pred(_covariance_results_solve);
  test_pred.left_multiply_transpose(K_train_test);
  //De-center the value and store for return
  Real mean_value = test_pred(0,0) + _training_mean(0,0);

  //Compute standard deviation
  DenseMatrix<Real> test_std = GaussianProcessTrainer::cholesky_back_substitute(_covariance_mat_cho_decomp, K_train_test);
  test_std.left_multiply_transpose(K_train_test);
  test_std.scale(-1);
  test_std += K_test;
  //Vairance computed, take sqrt for standard deviation and store
  *std_dev=std::sqrt(test_std(0,0));

  return mean_value;
}

// DenseMatrix<Real>
// GaussianProcess::cholesky_back_substitute(const DenseMatrix<Real> & A, const DenseMatrix<Real> & b)
// {
//   unsigned int n_cols=A.n();
//   //verify A.n == A.m == b.n
//   unsigned int n_solves=b.n();
//   //std::cout << n_cols << A.m() << b.n() << n_solves <<'\n';
//   DenseMatrix<Real> x(n_cols,n_solves);
//
//
//   //loosly modified
//   for (unsigned int s=0; s<n_solves; ++s){
//     // Solve for Ly=b
//     //std::cout << "s " << s << '\n';
//     for (unsigned int i=0; i<n_cols; ++i)
//     {
//       //std::cout << "i " << i << '\n';
//       Real temp = b(i,s);
//       for (unsigned int k=0; k<i; ++k){
//         temp -= A(i,k)*x(k,s);
//       }
//       x(i,s) = temp / A(i,i);
//     }
//
//     // Solve for L^T x = y
//     for (unsigned int i=0; i<n_cols; ++i)
//     {
//       //std::cout << "i " << i << '\n';
//       const unsigned int ib = (n_cols-1)-i;
//       for (unsigned int k=(ib+1); k<n_cols; ++k){
//         x(ib,s) -= A(k,ib) * x(k,s);
//       }
//       x(ib,s) /= A(ib,ib);
//     }
//   }
//   return x;
// }
