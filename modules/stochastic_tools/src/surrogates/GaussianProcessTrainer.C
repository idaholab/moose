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
    _covariance_results_solve(declareModelData<DenseMatrix<Real> >("_covariance_results_solve"))
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

void
GaussianProcessTrainer::execute()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  std::cout << "_n_params " << _n_params << '\n';
  // Initialize covariance matrix
  int _num_samples = _values_ptr->size();
  _covariance_mat.resize(_num_samples, _num_samples);

  std::cout << "_covariance_mat " << '\n' << _covariance_mat << '\n' << '\n';
  //

  //TODO:figure this out
  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  //load this into matrix for the time being, optimize later with proper Calls.
  //Arbitary access is helpfor for initial dev.
  //May load a very large matrix, which could be bad.
  _parameter_mat = _sampler->getLocalSamples();
  std::cout << "_parameter_mat " << '\n' << _parameter_mat << '\n' << '\n';

  //todo, implement scaling

  //Get training data
  _training_results.resize(_num_samples,1);

  std::cout << _values_ptr->at(0) << '\n';

  //populate covariance mat and load reasult data
  for (int ii=0; ii<_num_samples; ii++){
    for (int jj=0; jj<_num_samples; jj++){
        //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
        Real cov =0;
        for (int kk=0; kk<_n_params; kk++){
          //Compute distance per parameter
          std::cout << _parameter_mat(ii,kk) <<"   " << _parameter_mat(jj,kk) << "   " << _length_factor.at(kk) << '\n';
          cov += std::pow(( _parameter_mat(ii,kk)-  _parameter_mat(jj,kk) ),2) / (2.0 * _length_factor.at(kk));
        }
        cov = std::exp(-cov);
        _covariance_mat(ii,jj) = cov;
        _covariance_mat(jj,ii) = cov;
      }
    _training_results(ii,0) =   (*_values_ptr)[ii - offset];
  }

  _covariance_mat.print();
  // std::cout << "test 0" << '\n';
  // _training_results.print();


  //This is annoying, find better way to go between DenseMatrix and DenseVector
  DenseVector<Real> cho_solution_vec;
  DenseVector<Real> _training_results_vec(_num_samples);
  for (int ii=0; ii<_num_samples; ii++){
    _training_results_vec(ii)=_training_results(ii,0);
  }
  _covariance_results_solve.resize(_num_samples,1);
  DenseMatrix<Real> covariance_mat_copy(_covariance_mat);
  covariance_mat_copy.cholesky_solve(_training_results_vec,cho_solution_vec);
  for (int ii=0; ii<_num_samples; ii++){
    _covariance_results_solve(ii,0)=cho_solution_vec(ii);
    }
  std::cout << "After Cho Solve" << '\n';
   _covariance_mat.print();
   std::cout << "Training results"<< '\n';
   _training_results.print();
   std::cout << "Cho Solution" << '\n';
   _covariance_results_solve.print();

   // std::cout << "L?" << '\n';
   // covariance_mat_copy.print();
   //
   // std::cout << "A?" << '\n';
   // DenseMatrix<Real> another_covariance_mat_copy(covariance_mat_copy);
   // covariance_mat_copy.right_multiply_transpose(another_covariance_mat_copy);
   // covariance_mat_copy.print();


  // //A basic test of prediction ability. Move to surrogate after test
  // //
  // //
  // std::cout << "test A" << '\n';
  // Real _num_tests=2;
  // DenseMatrix<Real> _test_params(_num_tests,_n_params);
  // _test_params(0,0)=3;
  // _test_params(1,0)=5;
  // std::cout << "test B" << '\n';
  //
  // DenseMatrix<Real> K_train_test(_num_samples,_num_tests);
  // DenseMatrix<Real> K_test(_num_tests,_num_tests);
  //
  // std::cout << "test C" << '\n';
  //
  // for (int ii=0; ii<_num_samples; ii++){
  //   for (int jj=0; jj<_num_tests; jj++){
  //       //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
  //       Real val = std::pow(( _parameter_mat(ii,0)-  _test_params(jj,0)),2);
  //       val = val / (2.0 * _length_factor.at(0));
  //       val = std::exp(-val);
  //       K_train_test(ii,jj) = val;
  //     }
  // }
  //
  // K_train_test.print();
  //
  // for (int ii=0; ii<_num_tests; ii++){
  //   for (int jj=0; jj<_num_tests; jj++){
  //       //std::cout << _parameter_mat(ii,0) << "  "  << _parameter_mat(jj,0)  << '\n';
  //       Real val = std::pow(( _test_params(ii,0)-  _test_params(jj,0)),2);
  //       val = val / (2.0 * _length_factor.at(0));
  //       val = std::exp(-val);
  //       K_test(ii,jj) = val;
  //     }
  // }
  //
  // K_test.print();
  // std::cout << "Test Points" << '\n';
  // _parameter_mat.print();
  //
  // DenseMatrix<Real> _test_pred((_covariance_results_solve));
  // _test_pred.left_multiply_transpose(K_train_test);
  // std::cout << "pred" << '\n';
  // _test_pred.print();
}

void
GaussianProcessTrainer::finalize()
{
}
