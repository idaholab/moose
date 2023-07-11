//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialChaosTrainer.h"
#include "Sampler.h"
#include "CartesianProduct.h"

registerMooseObject("StochasticToolsApp", PolynomialChaosTrainer);

InputParameters
PolynomialChaosTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();
  params.addClassDescription("Computes and evaluates polynomial chaos surrogate model.");
  params.addRequiredParam<unsigned int>("order", "Maximum polynomial order.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");
  MooseEnum rtype("integration ols auto", "auto");
  params.addParam<MooseEnum>(
      "regression_type",
      rtype,
      "The type of regression to perform for finding polynomial coefficents.");
  params.addParam<Real>("penalty", 0.0, "Ridge regularization penalty factor for OLS regression.");

  params.suppressParameter<MooseEnum>("response_type");
  return params;
}

PolynomialChaosTrainer::PolynomialChaosTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _predictor_row(getPredictorData()),
    _order(declareModelData<unsigned int>("_order", getParam<unsigned int>("order"))),
    _ndim(declareModelData<unsigned int>("_ndim", _sampler.getNumberOfCols())),
    _tuple(declareModelData<std::vector<std::vector<unsigned int>>>(
        "_tuple", StochasticTools::MultiDimPolynomialGenerator::generateTuple(_ndim, _order))),
    _ncoeff(declareModelData<std::size_t>("_ncoeff", _tuple.size())),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _poly(declareModelData<std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>>>(
        "_poly")),
    _ridge_penalty(getParam<Real>("penalty")),
    _quad_sampler(dynamic_cast<QuadratureSampler *>(&_sampler))
{
  // Check if number of distributions is correct
  if (_ndim != _sampler.getNumberOfCols())
    paramError("distributions",
               "Sampler number of columns does not match number of inputted distributions.");

  auto rtype_enum = getParam<MooseEnum>("regression_type");
  if (rtype_enum == "auto")
    _rtype = _quad_sampler ? 0 : 1;
  else
    _rtype = rtype_enum == "integration" ? 0 : 1;

  if (_rtype == 0 && _quad_sampler &&
      (!_pvals.empty() || _pcols.size() != _sampler.getNumberOfCols()))
    paramError("sampler",
               "QuadratureSampler must use all Sampler columns for training, and cannot be"
               " used with other Reporters - otherwise, quadrature integration does not work.");
  if (_rtype == 0 && _ridge_penalty != 0.0)
    paramError("penalty",
               "Ridge regularization penalty is only relevant if 'regression_type = ols'.");

  // Make polynomials
  for (const auto & nm : getParam<std::vector<DistributionName>>("distributions"))
    _poly.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(nm)));

  // Create calculators for standardization
  if (_rtype == 1)
  {
    _calculators.resize(_ncoeff * 3);
    for (const auto & term : make_range(_ncoeff))
    {
      _calculators[3 * term] = StochasticTools::makeCalculator(MooseEnumItem("mean"), *this);
      _calculators[3 * term + 1] = StochasticTools::makeCalculator(MooseEnumItem("stddev"), *this);
      _calculators[3 * term + 2] = StochasticTools::makeCalculator(MooseEnumItem("sum"), *this);
    }
  }
}

void
PolynomialChaosTrainer::preTrain()
{
  _coeff.assign(_ncoeff, 0.0);

  if (_rtype == 1)
  {
    if (getCurrentSampleSize() < _ncoeff)
      paramError("order",
                 "Number of data points (",
                 getCurrentSampleSize(),
                 ") must be greater than the number of terms in the polynomial (",
                 _ncoeff,
                 ").");
    _matrix.resize(_ncoeff, _ncoeff);
    _rhs.resize(_ncoeff);
    for (auto & calc : _calculators)
      calc->initializeCalculator();
    _r_sum = 0.0;
  }
}

void
PolynomialChaosTrainer::train()
{
  // Evaluate polynomials to avoid duplication
  DenseMatrix<Real> poly_val(_ndim, _order);
  for (unsigned int d = 0; d < _ndim; ++d)
    for (unsigned int i = 0; i < _order; ++i)
      poly_val(d, i) = _poly[d]->compute(i, _predictor_row[d], /*normalize=*/_rtype == 0);

  // Evaluate multi-dimensional polynomials
  std::vector<Real> basis(_ncoeff, 1.0);
  for (const auto i : make_range(_ncoeff))
    for (const auto d : make_range(_ndim))
      basis[i] *= poly_val(d, _tuple[i][d]);

  // For integration
  if (_rtype == 0)
  {
    const Real fact = (*_rval) * (_quad_sampler ? _quad_sampler->getQuadratureWeight(_row) : 1.0);
    for (const auto i : make_range(_ncoeff))
      _coeff[i] += fact * basis[i];
  }
  // For least-squares
  else
  {
    // Loop over coefficients
    for (const auto i : make_range(_ncoeff))
    {
      // Matrix is symmetric, so we'll add the upper diagonal later
      for (const auto j : make_range(i + 1))
        _matrix(i, j) += basis[i] * basis[j];
      _rhs(i) += basis[i] * (*_rval);

      for (unsigned int c = i * 3; c < (i + 1) * 3; ++c)
        _calculators[c]->updateCalculator(basis[i]);
    }
    _r_sum += (*_rval);

    if (_ridge_penalty != 0.0)
      for (const auto i : make_range(_ncoeff))
        _matrix(i, i) += _ridge_penalty;
  }
}

void
PolynomialChaosTrainer::postTrain()
{
  if (_rtype == 0)
  {
    gatherSum(_coeff);
    if (!_quad_sampler)
      for (std::size_t i = 0; i < _ncoeff; ++i)
        _coeff[i] /= getCurrentSampleSize();
  }
  else
  {
    gatherSum(_matrix.get_values());
    gatherSum(_rhs.get_values());
    for (auto & calc : _calculators)
      calc->finalizeCalculator(true);
    gatherSum(_r_sum);

    std::vector<Real> mu(_ncoeff);
    std::vector<Real> sig(_ncoeff);
    std::vector<Real> sum_pf(_ncoeff);
    for (const auto i : make_range(_ncoeff))
    {
      mu[i] = i > 0 ? _calculators[3 * i]->getValue() : 0.0;
      sig[i] = i > 0 ? _calculators[3 * i + 1]->getValue() : 1.0;
      sum_pf[i] = _calculators[3 * i + 2]->getValue();
    }

    const Real n = getCurrentSampleSize();
    for (const auto i : make_range(_ncoeff))
    {
      for (const auto j : make_range(i + 1))
      {
        _matrix(i, j) -= (mu[j] * sum_pf[i] + mu[i] * sum_pf[j]);
        _matrix(i, j) += n * mu[i] * mu[j];
        _matrix(i, j) /= (sig[i] * sig[j]);
        _matrix(j, i) = _matrix(i, j);
      }
      _rhs(i) = (_rhs(i) - mu[i] * _r_sum) / sig[i];
    }

    DenseVector<Real> sol;
    _matrix.lu_solve(_rhs, sol);
    _coeff = sol.get_values();

    for (unsigned int i = 1; i < _ncoeff; ++i)
    {
      _coeff[i] /= sig[i];
      _coeff[0] -= _coeff[i] * mu[i];
    }
  }
}
