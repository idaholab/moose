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
  params.addRequiredParam<SamplerName>("sampler", "Training set defined by a sampler object.");
  params.addRequiredParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<unsigned int>("order", "Maximum polynomial order.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");

  return params;
}

PolynomialChaosTrainer::PolynomialChaosTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _order(declareModelData<unsigned int>("_order", getParam<unsigned int>("order"))),
    _ndim(declareModelData<unsigned int>("_ndim")),
    _ncoeff(declareModelData<std::size_t>("_ncoeff")),
    _tuple(declareModelData<std::vector<std::vector<unsigned int>>>("_tuple")),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _poly(declareModelData<std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>>>(
        "_poly"))
{
}

void
PolynomialChaosTrainer::initialize()
{
  _coeff.resize(_ncoeff, 0);
}

void
PolynomialChaosTrainer::initialSetup()
{
  // Setup data needed for training
  _tuple = StochasticTools::MultiDimPolynomialGenerator::generateTuple(
      getParam<std::vector<DistributionName>>("distributions").size(), _order);
  _ncoeff = _tuple.size();

  // Results VPP
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr = &getVectorPostprocessorValue(
      "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

  // Sampler
  _sampler = &getSamplerByName(getParam<SamplerName>("sampler"));
  _ndim = _sampler->getNumberOfCols();

  // Special circumstances if sampler is quadrature
  _quad_sampler = dynamic_cast<QuadratureSampler *>(_sampler);

  // Check if sampler dimension matches number of distributions
  std::vector<DistributionName> dname = getParam<std::vector<DistributionName>>("distributions");
  if (dname.size() != _ndim)
    mooseError("Sampler number of columns does not match number of inputted distributions.");
  for (auto nm : dname)
    _poly.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(nm)));
}

void
PolynomialChaosTrainer::execute()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  // Initialize storage for polynomial calculations
  DenseMatrix<Real> poly_val(_ndim, _order);

  // Offset for replicated/distributed result data
  dof_id_type offset = _values_distributed ? _sampler->getLocalRowBegin() : 0;

  // Loop over samples
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler->getNextLocalRow();

    // Evaluate polynomials to avoid duplication
    for (unsigned int d = 0; d < _ndim; ++d)
      for (unsigned int i = 0; i < _order; ++i)
        poly_val(d, i) = _poly[d]->compute(i, data[d]);

    // Loop over coefficients
    for (std::size_t i = 0; i < _ncoeff; ++i)
    {
      Real val = (*_values_ptr)[p - offset];
      // Loop over parameters
      for (std::size_t d = 0; d < _ndim; ++d)
        val *= poly_val(d, _tuple[i][d]);

      if (_quad_sampler)
        val *= _quad_sampler->getQuadratureWeight(p);
      _coeff[i] += val;
    }
  }
}

void
PolynomialChaosTrainer::finalize()
{
  gatherSum(_coeff);

  if (!_quad_sampler)
    for (std::size_t i = 0; i < _ncoeff; ++i)
      _coeff[i] /= _sampler->getNumberOfRows();
}
