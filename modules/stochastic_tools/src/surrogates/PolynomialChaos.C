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
#include "CartesianProduct.h"

registerMooseObject("StochasticToolsApp", PolynomialChaos);

InputParameters
PolynomialChaos::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates polynomial chaos surrogate model.");
  return params;
}

PolynomialChaos::PolynomialChaos(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _order(getModelData<unsigned int>("_order")),
    _ndim(getModelData<unsigned int>("_ndim")),
    _ncoeff(getModelData<std::size_t>("_ncoeff")),
    _tuple(getModelData<std::vector<std::vector<unsigned int>>>("_tuple")),
    _coeff(getModelData<std::vector<Real>>("_coeff")),
    _poly(
        getModelData<std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>>>("_poly"))
{
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
  for (std::size_t i = 0; i < _ncoeff; ++i)
  {
    Real tmp = _coeff[i];
    for (unsigned int d = 0; d < _ndim; ++d)
      tmp *= poly_val(d, _tuple[i][d]);
    val += tmp;
  }

  return val;
}

const std::vector<std::vector<unsigned int>> &
PolynomialChaos::getPolynomialOrders() const
{
  return _tuple;
}

unsigned
PolynomialChaos::getPolynomialOrder(const unsigned int dim, const unsigned int i) const
{
  return _tuple[i][dim];
}

const std::vector<Real> &
PolynomialChaos::getCoefficients() const
{
  return _coeff;
}

Real
PolynomialChaos::computeMean() const
{
  mooseAssert(_coeff.size() > 0, "The coefficient matrix is empty.");
  return _coeff[0];
}

Real
PolynomialChaos::computeStandardDeviation() const
{
  Real var = 0;
  for (std::size_t i = 1; i < _ncoeff; ++i)
  {
    Real norm = 1.0;
    for (std::size_t d = 0; d < _ndim; ++d)
      norm *= _poly[d]->innerProduct(_tuple[i][d]);
    var += _coeff[i] * _coeff[i] * norm;
  }

  return std::sqrt(var);
}

Real
PolynomialChaos::powerExpectation(const unsigned int n) const
{
  std::vector<StochasticTools::WeightedCartesianProduct<unsigned int, Real>> order;
  order.reserve(_ndim);
  std::vector<std::vector<Real>> c_1d(n, std::vector<Real>(_coeff.begin() + 1, _coeff.end()));
  for (unsigned int d = 0; d < _ndim; ++d)
  {
    std::vector<std::vector<unsigned int>> order_1d(n, std::vector<unsigned int>(_ncoeff - 1));
    for (std::size_t i = 1; i < _ncoeff; ++i)
      for (unsigned int m = 0; m < n; ++m)
        order_1d[m][i - 1] = _tuple[i][d];
    order.push_back(StochasticTools::WeightedCartesianProduct<unsigned int, Real>(order_1d, c_1d));
  }

  dof_id_type n_local, st_local, end_local;
  MooseUtils::linearPartitionItems(
      order[0].numRows(), n_processors(), processor_id(), n_local, st_local, end_local);

  Real val = 0;
  for (dof_id_type i = st_local; i < end_local; ++i)
  {
    Real tmp = order[0].computeWeight(i);
    for (unsigned int d = 0; d < _ndim; ++d)
    {
      if (MooseUtils::absoluteFuzzyEqual(tmp, 0.0))
        break;
      std::vector<unsigned int> comb(n);
      for (unsigned int m = 0; m < n; ++m)
        comb[m] = order[d].computeValue(i, m);
      tmp *= _poly[d]->productIntegral(comb);
    }
    val += tmp;
  }

  return val;
}

Real
PolynomialChaos::computeDerivative(const unsigned int dim, const std::vector<Real> & x) const
{
  return computePartialDerivative({dim}, x);
}

Real
PolynomialChaos::computePartialDerivative(const std::vector<unsigned int> & dim,
                                          const std::vector<Real> & x) const
{
  mooseAssert(x.size() == _ndim, "Number of inputted parameters does not match PC model.");

  std::vector<unsigned int> grad(_ndim);
  for (const auto & d : dim)
  {
    mooseAssert(d < _ndim, "Specified dimension is greater than total number of parameters.");
    grad[d]++;
  }

  DenseMatrix<Real> poly_val(_ndim, _order);

  // Evaluate polynomials to avoid duplication
  for (unsigned int d = 0; d < _ndim; ++d)
    for (unsigned int i = 0; i < _order; ++i)
      poly_val(d, i) = _poly[d]->computeDerivative(i, x[d], grad[d]);

  Real val = 0;
  for (std::size_t i = 0; i < _ncoeff; ++i)
  {
    Real tmp = _coeff[i];
    for (unsigned int d = 0; d < _ndim; ++d)
      tmp *= poly_val(d, _tuple[i][d]);
    val += tmp;
  }

  return val;
}

Real
PolynomialChaos::computeSobolIndex(const std::set<unsigned int> & ind) const
{
  linearPartitionCoefficients();

  // If set is empty, compute mean
  if (ind.empty())
    return computeMean();

  // Do some sanity checks in debug
  mooseAssert(ind.size() <= _ndim, "Number of indices is greater than number of parameters.");
  mooseAssert(*ind.rbegin() < _ndim, "Maximum index provided exceeds number of parameters.");

  Real val = 0.0;
  for (dof_id_type i = _local_coeff_begin; i < _local_coeff_end; ++i)
  {
    Real tmp = _coeff[i] * _coeff[i];
    for (unsigned int d = 0; d < _ndim; ++d)
    {
      if ((ind.find(d) != ind.end() && _tuple[i][d] > 0) ||
          (ind.find(d) == ind.end() && _tuple[i][d] == 0))
      {
        tmp *= _poly[d]->innerProduct(_tuple[i][d]);
      }
      else
      {
        tmp = 0.0;
        break;
      }
    }
    val += tmp;
  }

  return val;
}

Real
PolynomialChaos::computeSobolTotal(const unsigned int dim) const
{
  linearPartitionCoefficients();

  // Do some sanity checks in debug
  mooseAssert(dim < _ndim, "Requested dimension is greater than number of parameters.");

  Real val = 0.0;
  for (dof_id_type i = _local_coeff_begin; i < _local_coeff_end; ++i)
    if (_tuple[i][dim] > 0)
      val += _coeff[i] * _coeff[i] * _poly[dim]->innerProduct(_tuple[i][dim]);

  return val;
}

void
PolynomialChaos::linearPartitionCoefficients() const
{
  if (_n_local_coeff == std::numeric_limits<dof_id_type>::max())
    MooseUtils::linearPartitionItems(_ncoeff,
                                     n_processors(),
                                     processor_id(),
                                     _n_local_coeff,
                                     _local_coeff_begin,
                                     _local_coeff_end);
}

void
PolynomialChaos::store(nlohmann::json & json) const
{
  json["order"] = _order;
  json["ndim"] = getNumberOfParameters();
  json["ncoeff"] = getNumberofCoefficients();
  json["tuple"] = getPolynomialOrders();
  json["coeff"] = getCoefficients();
  for (const auto & p : _poly)
  {
    nlohmann::json jsonp;
    p->store(jsonp);
    json["poly"].push_back(jsonp);
  }
}
