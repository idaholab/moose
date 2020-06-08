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
  return params;
}

GaussianProcess::GaussianProcess(const InputParameters & parameters)
  : SurrogateModel(parameters),
  _sample_points(getModelData<std::vector<std::vector<Real>>>("_sample_points")),
  _length_factor(getModelData<std::vector<Real> >("_length_factor")),
  _parameter_mat(getModelData<DenseMatrix<Real> >("_parameter_mat")),
  _covariance_mat(getModelData<DenseMatrix<Real> >("_covariance_mat")),
  _training_results(getModelData<DenseMatrix<Real> >("_training_results")),
  _covariance_results_solve(getModelData<DenseMatrix<Real> >("_covariance_results_solve"))
{
  std::cout << "TEST IWH A" << '\n';
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x) const
{
  Real dummy=0;
  return this->evaluate(x, & dummy);
}

Real
GaussianProcess::evaluate(const std::vector<Real> & x, Real* std_dev) const
{
  //mooseAssert(x.size() == _ndim, "Number of inputted parameters does not match PC model.");

  //A basic test of prediction ability. Move to surrogate after test
  //
  //
  std::cout << "TEST IWH B" << '\n';

  int _n_params = _parameter_mat.n();
  int _num_samples = _parameter_mat.m();

  std::cout << "n_params " << _n_params << '\n';
  std::cout << "_num_samples " << _num_samples << '\n';
  for (int ii=0; ii<_n_params; ii++){
    std::cout << "x= " << x.at(ii) << '\n';
  }

  std::cout << "test A" << '\n';
  Real _num_tests=1;
  //DenseMatrix<Real> _test_params(_num_tests,_n_params);
  //_test_params(0,0)=3;
  //_test_params(1,0)=5;
  std::cout << "test B" << '\n';

  DenseMatrix<Real> K_train_test(_num_samples,_num_tests);
  DenseMatrix<Real> K_test(_num_tests,_num_tests);

  std::cout << "test C" << '\n';

  _parameter_mat.print();
  //_test_params.print();
  for (int ii=0; ii<_num_samples; ii++){
    for (int jj=0; jj<_num_tests; jj++){
      Real cov =0;
      for (int kk=0; kk<_n_params; kk++){
        std::cout << _parameter_mat(ii,kk) <<"   " << x.at(kk) << "   " << _length_factor.at(kk) << '\n';
        cov +=  std::pow(( _parameter_mat(ii,kk)-  x.at(kk)),2) / (2.0 * _length_factor.at(kk));
      }
      cov = std::exp(-cov);
      K_train_test(ii,jj) = cov;
      }
  }

  K_train_test.print();

  for (int ii=0; ii<_num_tests; ii++){
    for (int jj=0; jj<_num_tests; jj++){
      Real cov =0;
      for (int kk=0; kk<_n_params; kk++){
        std::cout << _parameter_mat(ii,kk) <<"   " << x.at(kk) << "   " << _length_factor.at(kk) << '\n';
        cov +=  std::pow(( _parameter_mat(ii,kk)-  x.at(kk)),2) / (2.0 * _length_factor.at(kk));
      }
      cov = std::exp(-cov);
      K_test(ii,jj) = cov;
      }
  }

  K_test.print();
  std::cout << "Training Points" << '\n';
  _parameter_mat.print();

  std::cout << "Training Data" << '\n';
  _training_results.print();



  DenseMatrix<Real> _test_pred(_covariance_results_solve);
  _test_pred.left_multiply_transpose(K_train_test);
  std::cout << "pred" << '\n';
  _test_pred.print();



  //This is annoying, find better way to go between DenseMatrix and DenseVector
  DenseMatrix<Real> _test_std(_num_samples,_num_tests);
  DenseVector<Real> cho_solution_vec;
  DenseVector<Real> K_train_test_vec(_num_samples);
  for (int ii=0; ii<_num_samples; ii++){
    K_train_test_vec(ii)=K_train_test(ii,0);
  }
  std::cout << "Covar" << '\n';
   _covariance_mat.print();
  DenseMatrix<Real> covariance_mat_copy(_covariance_mat);
  std::cout << "Covar Copy" << '\n';
   covariance_mat_copy.print();
  covariance_mat_copy.cholesky_solve(K_train_test_vec,cho_solution_vec);
  for (int ii=0; ii<_num_tests; ii++){
    _test_std(ii,0)=cho_solution_vec(ii);
    }
  std::cout << "STD After Cho Solve" << '\n';
   _test_std.print();
  //  std::cout << "Training results"<< '\n';
  //  _training_results.print();
  //  std::cout << "Cho Solutiol" << '\n';
  //  _covariance_results_solve.print();
  // DenseMatrix<Real> _test_std(_covariance_results_solve);
  _test_std.left_multiply_transpose(K_train_test);
  _test_std.scale(-1);
  _test_std += K_test;
  std::cout << "stddev" << '\n';
  _test_std.print();

  *std_dev=_test_std(0,0);

  return _test_pred(0,0);
}
