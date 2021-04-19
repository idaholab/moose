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
  params.addRequiredParam<ReporterName>(
      "response", "Reporter value of response results, can be vpp with <vpp_name>/<vector_name>.");
  params.addRequiredParam<unsigned int>("order", "Maximum polynomial order.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");

  return params;
}

PolynomialChaosTrainer::PolynomialChaosTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _pvals(getSamplerData()),
    _rval(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _order(declareModelData<unsigned int>("_order", getParam<unsigned int>("order"))),
    _ndim(declareModelData<unsigned int>("_ndim", _sampler.getNumberOfCols())),
    _tuple(declareModelData<std::vector<std::vector<unsigned int>>>(
        "_tuple", StochasticTools::MultiDimPolynomialGenerator::generateTuple(_ndim, _order))),
    _ncoeff(declareModelData<std::size_t>("_ncoeff", _tuple.size())),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _poly(declareModelData<std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>>>(
        "_poly")),
    _quad_sampler(dynamic_cast<QuadratureSampler *>(&_sampler))
{
  // Check if number of distributions is correct
  if (_ndim != _sampler.getNumberOfCols())
    paramError("distributions",
               "Sampler number of columns does not match number of inputted distributions.");

  // Make polynomials
  for (const auto & nm : getParam<std::vector<DistributionName>>("distributions"))
    _poly.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(nm)));

  _coeff.resize(_ncoeff, 0);
}

void
PolynomialChaosTrainer::train()
{
  DenseMatrix<Real> poly_val(_ndim, _order);

  // Evaluate polynomials to avoid duplication
  for (unsigned int d = 0; d < _ndim; ++d)
    for (unsigned int i = 0; i < _order; ++i)
      poly_val(d, i) = _poly[d]->compute(i, _pvals[d]);

  // Loop over coefficients
  for (std::size_t i = 0; i < _ncoeff; ++i)
  {
    Real val = _rval;
    // Loop over parameters
    for (std::size_t d = 0; d < _ndim; ++d)
      val *= poly_val(d, _tuple[i][d]);

    if (_quad_sampler)
      val *= _quad_sampler->getQuadratureWeight(_row);
    _coeff[i] += val;
  }
}

void
PolynomialChaosTrainer::postTrain()
{
  gatherSum(_coeff);

  if (!_quad_sampler)
    for (std::size_t i = 0; i < _ncoeff; ++i)
      _coeff[i] /= _sampler.getNumberOfRows();
}
