//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TwoLayerGaussianProcessSurrogate.h"
#include "Sampler.h"

#include "CovarianceFunctionBase.h"

registerMooseObject("StochasticToolsApp", TwoLayerGaussianProcessSurrogate);

InputParameters
TwoLayerGaussianProcessSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates Gaussian Process surrogate model.");
  return params;
}

TwoLayerGaussianProcessSurrogate::TwoLayerGaussianProcessSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    CovarianceInterface(parameters),
    _tgp(declareModelData<StochasticTools::TwoLayerGaussianProcess>("_tgp")),
    _training_params(getModelData<RealEigenMatrix>("_training_params"))
{
}

void
TwoLayerGaussianProcessSurrogate::setupCovariance(UserObjectName covar_name)
{
  if (_tgp.getCovarFunctionPtr() != nullptr)
    ::mooseError("Attempting to redefine covariance function using setupCovariance.");
  _tgp.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
}

Real
TwoLayerGaussianProcessSurrogate::evaluate(const std::vector<Real> & x) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  Real dummy = 0;
  return this->evaluate(x, dummy);
}

Real
TwoLayerGaussianProcessSurrogate::evaluate(const std::vector<Real> & x, Real & std_dev) const
{
  std::vector<Real> y;
  std::vector<Real> std;
  this->evaluate(x, y, std);
  std_dev = std[0];
  return y[0];
}

void
TwoLayerGaussianProcessSurrogate::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Overlaod for evaluate to maintain general compatibility. Only returns mean
  std::vector<Real> std_dummy;
  this->evaluate(x, y, std_dummy);
}

// void 
// TwoLayerGaussianProcessSurrogate::squared_exponential_covariance(const RealEigenMatrix &x1, 
//                   const RealEigenMatrix &x2, 
//                   Real tau2, 
//                   const RealEigenMatrix &theta, 
//                   Real g, 
//                   RealEigenMatrix &k)
// {
//   int n1 = x1.rows();
//   int n2 = x2.rows();
//   // k.resize(n1, n2);
//   // std::cout << "x1 rows is " << x1.rows() << std::endl;
//   // std::cout << "x1 cols is " << x1.cols() << std::endl;
  
//   for (int i = 0; i < n1; ++i) {
//     for (int j = 0; j < n2; ++j) {
//       // Compute the scaled distance r_l(x1, x2)
//       Eigen::RowVectorXd diff = (x1.row(i) - x2.row(j)).array() / theta.row(0).array();
//       Real r_l = std::sqrt(diff.squaredNorm());
//       Real cov_val = tau2 * std::exp(-0.5 * r_l * r_l);
//       if (i == j) {
//           cov_val += g;
//       }
//       k(i, j) = cov_val;
//     }
//   }
//   // std::cout << "k is " << k << std::endl;
// }

// void
// TwoLayerGaussianProcessSurrogate::krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
//                                    const RealEigenMatrix & theta, Real g, Real tau2, bool cal_sigma,
//                                    const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new, KrigResult & result)
// {
//   RealEigenMatrix C;
//   RealEigenMatrix C_cross;
//   RealEigenMatrix C_new;
//   squared_exponential_covariance(x, x, tau2, theta, g, C);
//   squared_exponential_covariance(x, x_new, tau2, theta, g, C_cross);

//   Eigen::LLT<RealEigenMatrix> llt(C);
//   RealEigenMatrix C_inv = llt.solve(RealEigenMatrix::Identity(C.rows(), C.cols()));

//   // RealEigenMatrix L = llt.matrixL();
//   // Real ldet = 2 * L.diagonal().array().log().sum();
//   // result.Mi = Mi;
//   // result.ldet = ldet;

//   // RealEigenMatrix mean = prior_mean_new + C_cross * C_inv * (y - prior_mean);
//   RealEigenMatrix mean =  C_cross * C_inv * (y - prior_mean);
//   RealEigenMatrix sigma;

//   if (cal_sigma) {
//     RealEigenMatrix quad_term = C_cross * C_inv * C_cross.transpose();
//     squared_exponential_covariance(x_new, x_new, tau2, theta, g, C_new);
//     sigma = tau2 * (C_new - quad_term);
//   }

//   result.mean = mean;
//   result.sigma = sigma;

// }

// void
// TwoLayerGaussianProcessSurrogate::predict(const RealEigenMatrix & x_new)
// {
//   RealEigenMatrix x = _training_params;
//   RealEigenMatrix prior_mean_new = RealEigenMatrix::Zero(1, 1);;
//   RealEigenMatrix prior_mean = RealEigenMatrix::Zero(_training_params.rows(), 1);
//   Real prior_tau2 = 1;
//   RealEigenMatrix mu_t = RealEigenMatrix::Zero(1, _tgp.getNmcmc());
//   RealEigenMatrix sigma_sum(1, 1);
//   RealEigenMatrix w_t;
//   RealEigenMatrix w_new(1, _training_params.cols());
//   KrigResult krig_result;
//   RealEigenMatrix theta_w(1, _training_params.cols());

//   for (unsigned int t = 0; t < _tgp.getNmcmc(); t++){
//     w_t = _tgp.getW()[t];
//     w_new = RealEigenMatrix::Zero(1, _training_params.cols());
//     for (unsigned int i = 0; t < _training_params.cols(); i++){
//       // std::cout << "test points: " << test_points << "," << _tgp.getW()[0] << "," << _tgp.getG() << "," << _tgp.getThetaY() << "," << _tgp.getThetaW() << "," << _tgp.getTau2() << std::endl;
// // krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
// //                                    Real theta, Real g, Real tau2, bool cal_sigma,
// //                                    const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new,KrigResult & result)

//       theta_w = RealEigenMatrix::Constant(1, _training_params.cols(), _tgp.getThetaW()(t,i));
//       krig(w_t.col(i), x, x_new, theta_w, 1e-10, prior_tau2, false, prior_mean, prior_mean_new, krig_result);
//       w_new.col(i) = krig_result.mean;
//     }
//     krig(_tgp.getY(), _tgp.getW()[t], w_new, _tgp.getThetaY().row(t), _tgp.getG()(t,0), _tgp.getTau2()(t,0), true, prior_mean, prior_mean_new, krig_result);
//     mu_t.col(t) = krig_result.mean;
//     sigma_sum += krig_result.sigma;
//   }
//   RealEigenMatrix mean = mu_t.rowwise().mean();
//   std::cout << "mean is: " << mean << "," << mu_t.rows() << "," << mu_t.cols() << "," << mean.rows() << "," << mean.cols() << std::endl;
//   // RealEigenMatrix mu_t_centered = mu_t.rowwise() - mean.transpose();
//   // RealEigenMatrix covariance = (mu_t_centered.adjoint() * mu_t_centered) / (_tgp.getNmcmc() - 1);
//   // RealEigenMatrix Sigma = (sigma_sum / _tgp.getNmcmc()) + covariance;

// }

void
TwoLayerGaussianProcessSurrogate::evaluate(const std::vector<Real> & x,
                                   std::vector<Real> & y,
                                   std::vector<Real> & std) const
{
  
  const unsigned int n_dims = _training_params.cols();

  mooseAssert(x.size() == n_dims,
              "Number of parameters provided for evaluation does not match number of parameters "
              "used for training.");
  const unsigned int n_outputs = _tgp.getCovarFunction().numOutputs();

  y = std::vector<Real>(n_outputs, 0.0);
  std = std::vector<Real>(n_outputs, 0.0);

  RealEigenMatrix test_points(1, n_dims);
  for (unsigned int ii = 0; ii < n_dims; ++ii)
    test_points(0, ii) = x[ii];

  _tgp.getParamStandardizer().getStandardized(test_points);
  // std::cout << "test points: " << test_points << "," << _tgp.getW()[0] << "," << _tgp.getG() << "," << _tgp.getThetaY() << "," << _tgp.getThetaW() << "," << _tgp.getTau2() << std::endl;
  // std::cout << "test points: " << "," << _training_params << "," << _tgp.getX() << "," << _tgp.getY() << std::endl;
  // predict(test_points);

  RealEigenMatrix x_old = _training_params;
  RealEigenMatrix prior_mean_new = RealEigenMatrix::Zero(1, 1);;
  RealEigenMatrix prior_mean = RealEigenMatrix::Zero(_training_params.rows(), 1);
  Real prior_tau2 = 1;
  RealEigenMatrix mu_t = RealEigenMatrix::Zero(1, _tgp.getNmcmc());
  RealEigenMatrix sigma_sum(1, 1);
  RealEigenMatrix w_t;
  RealEigenMatrix w_new(1, _training_params.cols());
  RealEigenMatrix theta_w(1, _training_params.cols());
  RealEigenMatrix krig_mean;
  RealEigenMatrix krig_sigma;

  for (unsigned int t = 0; t < _tgp.getNmcmc(); t++){
    w_t = _tgp.getW()[t];
    w_new = RealEigenMatrix::Zero(1, _training_params.cols());
    for (unsigned int i = 0; i < _training_params.cols(); i++){
      // std::cout << "test points: " << test_points << "," << _tgp.getW()[0] << "," << _tgp.getG() << "," << _tgp.getThetaY() << "," << _tgp.getThetaW() << "," << _tgp.getTau2() << std::endl;
// krig(const RealEigenMatrix & y, const RealEigenMatrix & x, const RealEigenMatrix & x_new,
//                                    Real theta, Real g, Real tau2, bool cal_sigma,
//                                    const RealEigenMatrix & prior_mean, const RealEigenMatrix & prior_mean_new,KrigResult & result)
      // std::cout << "constant theta w0 is : " << _tgp.getNmcmc() << ", " << _training_params.cols() << ", " << _tgp.getThetaW().rows() << ", " << _tgp.getThetaW().cols() << std::endl;
      // std::cout << "constant theta w1 is : " << _training_params.cols() << "," << t << ", " << i << std::endl;
      // std::cout << "constant theta w2 is : " << _tgp.getThetaW()(t,i) << std::endl;
      theta_w = RealEigenMatrix::Constant(1, _training_params.cols(), _tgp.getLengthscaleW()(t,i));
      _tgp.krig(w_t.col(i), x_old, test_points, theta_w, 1e-10, prior_tau2, false, prior_mean, prior_mean_new, krig_mean, krig_sigma);
      w_new.col(i) = krig_mean;
    }
    _tgp.krig(_tgp.getY(), _tgp.getW()[t], w_new, _tgp.getLengthscaleY().row(t), _tgp.getNoise()(t,0), _tgp.getScale()(t,0), true, prior_mean, prior_mean_new, krig_mean, krig_sigma);
    mu_t.col(t) = krig_mean;
    sigma_sum += krig_sigma;
  }
  // RealEigenMatrix mean = mu_t.rowwise().mean();
  RealEigenMatrix mean = RealEigenMatrix::Constant(1, mu_t.cols(), mu_t.rowwise().mean()(0,0));
  // std::cout << "mean is: " << mean(0,0) << "," << mu_t.rows() << "," << mu_t.cols() << "," << mean.rows() << "," << mean.cols() << std::endl;
  // 1,10000,1,1
  RealEigenMatrix mu_t_centered = mu_t - mean;
  // std::cout << "mean center is: " << mu_t_centered << "," << mu_t_centered.rows() << "," << mu_t_centered.cols() << std::endl;
  RealEigenMatrix covariance = (mu_t_centered * mu_t_centered.transpose()) / (_tgp.getNmcmc() - 1);
  // std::cout << "covariance is: " << covariance << "," << covariance.rows() << "," << covariance.cols() << std::endl;
  RealEigenMatrix Sigma = (sigma_sum / _tgp.getNmcmc()) + covariance;
  // std::cout << "sigma is: " << Sigma << "," << Sigma.rows() << "," << Sigma.cols() << ", " << sigma_sum << std::endl;
  RealEigenMatrix pred_value = mean.row(0).col(0);
  _tgp.getDataStandardizer().getDestandardized(pred_value);
  RealEigenMatrix pred_var = Sigma;
  RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  _tgp.getDataStandardizer().getDescaled(std_dev_mat);

  for (const auto output_i : make_range(n_outputs))
  {
    y[output_i] = pred_value(0, output_i);
    std[output_i] = std_dev_mat(output_i, output_i);
  }







  // RealEigenMatrix K_train_test(_training_params.rows() * n_outputs, n_outputs);

  // _tgp.getCovarFunction().computeCovarianceMatrix(
  //     K_train_test, _training_params, test_points, false);
  // RealEigenMatrix K_test(n_outputs, n_outputs);
  // _tgp.getCovarFunction().computeCovarianceMatrix(K_test, test_points, test_points, true);

  // // Compute the predicted mean value (centered)
  // RealEigenMatrix pred_value = (K_train_test.transpose() * _tgp.getKResultsSolve()).transpose();
  // // De-center/scale the value and store for return
  // // std::cout << "pred_value: " << pred_value << std::endl;
  // _tgp.getDataStandardizer().getDestandardized(pred_value);

  // RealEigenMatrix pred_var =
  //     K_test - (K_train_test.transpose() * _tgp.getKCholeskyDecomp().solve(K_train_test));

  // // std::cout << "enter twolayer prediction" << std::endl;
  // // Vairance computed, take sqrt for standard deviation, scale up by training data std and store
  // RealEigenMatrix std_dev_mat = pred_var.array().sqrt();
  // _tgp.getDataStandardizer().getDescaled(std_dev_mat);

  // for (const auto output_i : make_range(n_outputs))
  // {
  //   y[output_i] = pred_value(0, output_i);
  //   std[output_i] = std_dev_mat(output_i, output_i);
  // }
}
