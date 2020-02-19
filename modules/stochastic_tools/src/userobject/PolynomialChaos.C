//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialChaos.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", PolynomialChaos);

defineLegacyParams(PolynomialChaos);

InputParameters
PolynomialChaos::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates polynomial chaos surrogate model.");

  params.addRequiredParam<unsigned int>("order", "Maximum polynomial order.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");

  return params;
}

PolynomialChaos::PolynomialChaos(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _order(getParam<unsigned int>("order")),
    _quad_sampler(nullptr),
    _tuple(generateTuple(getParam<std::vector<DistributionName>>("distributions").size(), _order)),
    _ncoeff(_tuple.size()),
    _coeff(_ncoeff)
{
}

void
PolynomialChaos::initialSetup()
{
  SurrogateModel::initialSetup();

  // Special circumstances if sampler is quadrature
  _quad_sampler = dynamic_cast<QuadratureSampler *>(_sampler);

  std::vector<DistributionName> dname = getParam<std::vector<DistributionName>>("distributions");
  // Check if sampler dimensionality matches number of distributions
  if (dname.size() != _ndim)
    mooseError("Sampler number of columns does not match number of inputted distributions.");
  for (auto nm : dname)
    _poly.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(nm)));
}

void
PolynomialChaos::execute()
{
  // Check if results of samples matches number of samples
  mooseAssert(_sampler->getNumberOfRows() == _values.size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  std::fill(_coeff.begin(), _coeff.end(), 0.0);
  DenseMatrix<Real> poly_val(_ndim, _order);

  // Loop over samples
  for (dof_id_type p = _sampler->getLocalRowBegin(); p < _sampler->getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler->getNextLocalRow();

    // Evaluate polynomials to avoid duplication
    for (unsigned int d = 0; d < _ndim; ++d)
      for (unsigned int i = 0; i < _order; ++i)
        poly_val(d, i) = _poly[d]->compute(i, data[d]);

    // Loop over coefficients
    for (unsigned int i = 0; i < _ncoeff; ++i)
    {
      Real val = _values[p - _sampler->getLocalRowBegin()];
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
PolynomialChaos::finalize()
{
  gatherSum(_coeff);

  if (!_quad_sampler)
    for (unsigned int i = 0; i < _ncoeff; ++i)
      _coeff[i] /= _sampler->getNumberOfRows();
}

void
PolynomialChaos::threadJoin(const UserObject & y)
{
  const PolynomialChaos & pps = static_cast<const PolynomialChaos &>(y);
  for (unsigned int i = 0; i < _ncoeff; ++i)
    _coeff[i] += pps._coeff[i];
}

Real
PolynomialChaos::evaluate(const std::vector<Real> & x) const
{

  mooseAssert(x.size() == _ndim, "Number of inputted parameters does not match PC model.");

  DenseMatrix<Real> poly_val(_ndim, _order);

  // Evaluate polynomials to avoid duplication
  for (unsigned int d = 0; d < _ndim; ++d)
    for (unsigned int i = 0; i < _order; ++i)
      poly_val(d, i) = _poly[d]->compute(i, x[d], /*normalize =*/false);

  Real val = 0;
  for (unsigned int i = 0; i < _ncoeff; ++i)
  {
    Real tmp = _coeff[i];
    for (unsigned int d = 0; d < _ndim; ++d)
      tmp *= poly_val(d, _tuple[i][d]);
    val += tmp;
  }

  return val;
}

std::vector<std::vector<unsigned int>>
PolynomialChaos::generateTuple(const unsigned int ndim, const unsigned int order)
{
  // Compute full tensor tuple
  std::vector<std::vector<unsigned int>> tuple_1d(ndim);
  for (unsigned int d = 0; d < ndim; ++d)
  {
    tuple_1d[d].resize(order);
    for (unsigned int i = 0; i < order; ++i)
      tuple_1d[d][i] = i;
  }
  StochasticTools::CartesianProduct<unsigned int> tensor_tuple(tuple_1d);

  // Remove polynomials that exceed the maximum order
  std::vector<std::vector<unsigned int>> tuple;
  for (unsigned int p = 0; p < tensor_tuple.numRows(); ++p)
  {
    std::vector<unsigned int> dorder = tensor_tuple.computeRow(p);
    unsigned int sum = std::accumulate(dorder.begin(), dorder.end(), 0);
    if (sum < order)
      tuple.push_back(dorder);
  }

  std::sort(tuple.begin(), tuple.end(), sortTuple);
  return tuple;
}

bool
PolynomialChaos::sortTuple(const std::vector<unsigned int> & first,
                           const std::vector<unsigned int> & second)
{
  // Sort by sum
  if (std::accumulate(first.begin(), first.end(), 0) <
      std::accumulate(second.begin(), second.end(), 0))
    return true;
  else if (std::accumulate(first.begin(), first.end(), 0) >
           std::accumulate(second.begin(), second.end(), 0))
    return false;

  // Loop over elements
  for (unsigned int d = 0; d < first.size(); ++d)
  {
    if (first[d] == second[d])
      continue;
    return (first[d] > second[d]);
  }

  return false;
}
