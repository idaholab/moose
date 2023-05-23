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

  MooseEnum rtype("ols=0 ridge=1");
  params.addRequiredParam<MooseEnum>(
      "regression_type", rtype, "The type of regression to perform.");
  params.addRequiredParam<unsigned int>("max_degree",
                                        "Maximum polynomial degree to use for the regression.");
  params.addParam<Real>("penalty", 0.0, "Penalty for Ridge regularization.");

  return params;
}

PolynomialRegressionTrainer::PolynomialRegressionTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters), // SurrogateTrainer(parameters),
    _predictor_row(getPredictorData()),
    _regression_type(getParam<MooseEnum>("regression_type")),
    _penalty(getParam<Real>("penalty")),
    _coeff(declareModelData<std::vector<std::vector<Real>>>("_coeff")),
    _max_degree(
        declareModelData<unsigned int>("_max_degree", getParam<unsigned int>("max_degree"))),
    _power_matrix(declareModelData<std::vector<std::vector<unsigned int>>>(
        "_power_matrix",
        StochasticTools::MultiDimPolynomialGenerator::generateTuple(_n_dims, _max_degree + 1))),
    _n_poly_terms(_power_matrix.size()),
    _matrix(_n_poly_terms, _n_poly_terms),
    _rhs(1, DenseVector<Real>(_n_poly_terms, 0.0)),
    _r_sum(1, 0.0)
{
  _coeff.resize(_n_poly_terms);

  // Throwing a warning if the penalty parameter is set with OLS regression
  if (_regression_type == "ols" && _penalty != 0)
    mooseWarning("Penalty parameter is not used for OLS regression, found penalty=", _penalty);

  // Check if we have enough data points to solve the problem
  if (_sampler.getNumberOfRows() <= _n_poly_terms)
    mooseError("Number of data points must be greater than the number of terms in the polynomial.");

  // Creating calculators needed for feature standardization.
  _calculators.resize(_n_poly_terms * 3);
  for (const auto & term : make_range(_n_poly_terms))
  {
    _calculators[3 * term] = StochasticTools::makeCalculator(MooseEnumItem("mean"), *this);
    _calculators[3 * term + 1] = StochasticTools::makeCalculator(MooseEnumItem("stddev"), *this);
    _calculators[3 * term + 2] = StochasticTools::makeCalculator(MooseEnumItem("sum"), *this);
  }
}

void
PolynomialRegressionTrainer::preTrain()
{
  _matrix.zero();
  for (unsigned int r = 0; r < _rhs.size(); ++r)
    _rhs[r].zero();

  /// Init calculators.
  for (auto & calc : _calculators)
    calc->initializeCalculator();

  _r_sum.assign(_rhs.size(), 0.0);
}

void
PolynomialRegressionTrainer::train()
{
  // Caching the different powers of data to accelerate the assembly of the
  // system
  DenseMatrix<Real> data_pow(_n_dims, _max_degree + 1);
  for (unsigned int d = 0; d < _n_dims; ++d)
    for (unsigned int i = 0; i <= _max_degree; ++i)
      data_pow(d, i) = pow(_predictor_row[d], i);

  // Emplace new values if necessary
  if (_rvecval)
    for (unsigned int r = _rhs.size(); r < _rvecval->size(); ++r)
    {
      _rhs.emplace_back(_n_poly_terms, 0.0);
      _r_sum.emplace_back(0.0);
    }

  for (unsigned int i = 0; i < _n_poly_terms; ++i)
  {
    Real i_value(1.0);
    for (unsigned int ii = 0; ii < _n_dims; ++ii)
      i_value *= data_pow(ii, _power_matrix[i][ii]);

    for (unsigned int j = 0; j < _n_poly_terms; ++j)
    {
      Real j_value(1.0);
      for (unsigned int jj = 0; jj < _n_dims; ++jj)
        j_value *= data_pow(jj, _power_matrix[j][jj]);

      _matrix(i, j) += i_value * j_value;
    }

    // Update calculators.
    for (unsigned int c = i * 3; c < (i + 1) * 3; ++c)
      _calculators[c]->updateCalculator(i_value);

    if (_rval)
      _rhs[0](i) += i_value * (*_rval);
    else if (_rvecval)
      for (unsigned int r = 0; r < _rvecval->size(); ++r)
        _rhs[r](i) += i_value * (*_rvecval)[r];
  }

  if (_rval)
    _r_sum[0] += (*_rval);
  else if (_rvecval)
    for (unsigned int r = 0; r < _rvecval->size(); ++r)
      _r_sum[r] += (*_rvecval)[r];

  // Adding penalty term for Ridge regularization
  if (_regression_type == "ridge")
    for (unsigned int i = 0; i < _n_poly_terms; ++i)
      _matrix(i, i) += _penalty;
}

void
PolynomialRegressionTrainer::postTrain()
{
  // Make sure _rhs are all the same size
  unsigned int nrval = _rhs.size();
  gatherMax(nrval);
  for (unsigned int r = _rhs.size(); r < nrval; ++r)
  {
    _rhs.emplace_back(_n_poly_terms, 0.0);
    _r_sum.emplace_back(0.0);
  }

  // Gather regression data
  gatherSum(_matrix.get_values());
  for (auto & it : _rhs)
    gatherSum(it.get_values());

  // Gather response sums.
  gatherSum(_r_sum);

  // Finalize calculators.
  for (auto & calc : _calculators)
    calc->finalizeCalculator(true);

  std::vector<Real> mu(_n_poly_terms);
  std::vector<Real> sig(_n_poly_terms);
  std::vector<Real> sum_pf(_n_poly_terms);

  for (unsigned int i = 0; i < _n_poly_terms; ++i)
  {
    // To handle intercept, use mu = 0, sig = 1.
    mu[i] = i > 0 ? _calculators[3 * i]->getValue() : 0.0;
    sig[i] = i > 0 ? _calculators[3 * i + 1]->getValue() : 1.0;
    sum_pf[i] = _calculators[3 * i + 2]->getValue();
  }

  // Transform _matrix and _rhs to match standardized features.
  const Real n = sum_pf[0]; // Sum of intercept term = n.
  for (unsigned int i = 0; i < _n_poly_terms; ++i)
  {
    for (unsigned int j = 0; j < _n_poly_terms; ++j)
    {
      _matrix(i, j) -= (mu[j] * sum_pf[i] + mu[i] * sum_pf[j]);
      _matrix(i, j) += n * mu[i] * mu[j];
      _matrix(i, j) /= (sig[i] * sig[j]);
    }
    for (unsigned int r = 0; r < _rhs.size(); ++r)
      _rhs[r](i) = (_rhs[r](i) - mu[i] * _r_sum[r]) / sig[i];
  }

  DenseVector<Real> sol;

  _coeff.resize(_rhs.size());
  for (unsigned int r = 0; r < _rhs.size(); ++r)
  {
    _matrix.lu_solve(_rhs[r], sol);
    _coeff[r] = sol.get_values();

    // Transform coefficients to match unstandardized features.
    for (unsigned int i = 1; i < _n_poly_terms; ++i)
    {
      _coeff[r][i] /= sig[i];
      _coeff[r][0] -= _coeff[r][i] * mu[i];
    }
  }
}
