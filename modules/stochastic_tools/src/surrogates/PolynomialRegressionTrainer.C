//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialRegressionTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", PolynomialRegressionTrainer);

InputParameters
PolynomialRegressionTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Computes coefficients for polynomial regession model.");

  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  MooseEnum rtype("ols=0 ridge=1");
  params.addRequiredParam<MooseEnum>(
      "regression_type", rtype, "The type of regression to perform.");
  params.addRequiredParam<unsigned int>("max_degree",
                                        "Maximum polynomial degree to use for the regression.");
  params.addParam<Real>("penalty", 0.0, "Penalty for Ridge regularization.");

  return params;
}

PolynomialRegressionTrainer::PolynomialRegressionTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _rval(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _n_dims((_pvals.empty() && _pcols.empty()) ? _sampler.getNumberOfCols()
                                               : (_pvals.size() + _pcols.size())),
    _regression_type(getParam<MooseEnum>("regression_type")),
    _penalty(getParam<Real>("penalty")),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _max_degree(
        declareModelData<unsigned int>("_max_degree", getParam<unsigned int>("max_degree"))),
    _power_matrix(declareModelData<std::vector<std::vector<unsigned int>>>(
        "_power_matrix",
        StochasticTools::MultiDimPolynomialGenerator::generateTuple(_n_dims, _max_degree + 1))),
    _n_poly_terms(_power_matrix.size()),
    _matrix(_n_poly_terms, _n_poly_terms),
    _rhs(_n_poly_terms)
{
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

  _coeff.resize(_n_poly_terms);

  // Throwing a warning if the penalty parameter is set with OLS regression
  if (_regression_type == "ols" && _penalty != 0)
    mooseWarning("Penalty parameter is not used for OLS regression, found penalty=", _penalty);

  // Check if we have enough data points to solve the problem
  if (_sampler.getNumberOfRows() <= _n_poly_terms)
    mooseError("Number of data points must be greater than the number of terms in the polynomial.");
}

void
PolynomialRegressionTrainer::train()
{
  // Caching the different powers of data to accelerate the assembly of the
  // system
  DenseMatrix<Real> data_pow(_n_dims, _max_degree + 1);
  for (unsigned int d = 0; d < _n_dims; ++d)
  {
    const Real val = d < _pvals.size() ? *_pvals[d] : _sampler_row[_pcols[d - _pvals.size()]];
    for (unsigned int i = 0; i <= _max_degree; ++i)
      data_pow(d, i) = pow(val, i);
  }

  for (unsigned int i = 0; i < _n_poly_terms; ++i)
  {
    std::vector<unsigned int> i_powers(_power_matrix[i]);

    Real i_value(1.0);
    for (unsigned int ii = 0; ii < _n_dims; ++ii)
      i_value *= data_pow(ii, i_powers[ii]);

    for (unsigned int j = 0; j < _n_poly_terms; ++j)
    {
      std::vector<unsigned int> j_powers(_power_matrix[j]);

      Real j_value(1.0);
      for (unsigned int jj = 0; jj < _n_dims; ++jj)
        j_value *= data_pow(jj, j_powers[jj]);

      _matrix(i, j) += i_value * j_value;
    }

    _rhs(i) += i_value * _rval;
  }

  // Adding penalty term for Ridge regularization
  if (_regression_type == "ridge")
    for (unsigned int i = 0; i < _n_poly_terms; ++i)
      _matrix(i, i) += _penalty;
}

void
PolynomialRegressionTrainer::postTrain()
{
  gatherSum(_matrix.get_values());
  gatherSum(_rhs.get_values());

  DenseVector<Real> sol;
  _matrix.lu_solve(_rhs, sol);
  _coeff = sol.get_values();
}
