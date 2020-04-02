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

#include "SerializerGuard.h"

registerMooseObject("StochasticToolsApp", PolynomialChaos);

InputParameters
PolynomialChaos::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Computes and evaluates polynomial chaos surrogate model.");

  // Training parameters
  params.addParam<SamplerName>("training_sampler", "Training set defined by a sampler object.");
  params.addParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addParam<std::string>(
      "results_vector",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addParam<unsigned int>("order", "Maximum polynomial order.");
  params.addParam<std::vector<DistributionName>>(
      "distributions", "Names of the distributions samples were taken from.");

  params.addParamNamesToGroup("training_sampler results_vpp results_vector order distributions",
                              "training");
  return params;
}

PolynomialChaos::PolynomialChaos(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _order(declareModelData<unsigned int>("_order", getParam<unsigned int>("order")),
    _tuple(declareModelData<std::vector<std::vector<unsigned int>>>("_tuple")),
    _coeff(declareModelData<std::vector<Real>>("_coeff")),
    _poly(declareModelData<std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>>>(
        "_poly"))
{
}

void
PolynomialChaos::initialSetup()
{
  if (isTraining())
  {
    // Setup data needed for training
    _tuple = generateTuple(getParam<std::vector<DistributionName>>("distributions").size(), _order);
    _ncoeff = _tuple.size();
    _coeff.resize(_ncoeff, 0);

    // Results VPP
    _values_distributed = isVectorPostprocessorDistributed("results_vpp");
    _values_ptr = &getVectorPostprocessorValue(
        "results_vpp", getParam<std::string>("results_vector"), !_values_distributed);

    // Sampler
    _sampler = &getSamplerByName(getParam<SamplerName>("training_sampler"));
    _ndim = _sampler->getNumberOfCols();

    // Special circumstances if sampler is quadrature
    _quad_sampler = dynamic_cast<QuadratureSampler *>(_sampler);

    // Check if sampler dimensionality matches number of distributions
    std::vector<DistributionName> dname = getParam<std::vector<DistributionName>>("distributions");
    if (dname.size() != _ndim)
      mooseError("Sampler number of columns does not match number of inputted distributions.");
    for (auto nm : dname)
      _poly.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(nm)));
  }

  // evaluate
  else
  {
    _ncoeff = _tuple.size();
    _ndim = _poly.size();
  }

  // Setup parallel partitioning of coefficients
  MooseUtils::linearPartitionItems(_ncoeff,
                                   n_processors(),
                                   processor_id(),
                                   _n_local_coeff,
                                   _local_coeff_begin,
                                   _local_coeff_end);
}

void
PolynomialChaos::train()
{
  // Check if results of samples matches number of samples
  __attribute__((unused)) dof_id_type num_rows =
      _values_distributed ? _sampler->getNumberOfLocalRows() : _sampler->getNumberOfRows();
  mooseAssert(num_rows == _values_ptr->size(),
              "Sampler number of rows does not match number of results from vector postprocessor.");

  std::fill(_coeff.begin(), _coeff.end(), 0.0);
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
PolynomialChaos::trainFinalize()
{
  gatherSum(_coeff);

  if (!_quad_sampler)
    for (std::size_t i = 0; i < _ncoeff; ++i)
      _coeff[i] /= _sampler->getNumberOfRows();
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

unsigned int
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
PolynomialChaos::powerExpectation(const unsigned int n, const bool distributed) const
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

  dof_id_type n_local = order[0].numRows();
  dof_id_type st_local = 0;
  dof_id_type end_local = n_local;
  if (distributed)
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
  // Do some sanity checks in debug
  mooseAssert(dim < _ndim, "Requested dimesion is greater than number of parameters.");

  Real val = 0.0;
  for (dof_id_type i = _local_coeff_begin; i < _local_coeff_end; ++i)
    if (_tuple[i][dim] > 0)
      val += _coeff[i] * _coeff[i] * _poly[dim]->innerProduct(_tuple[i][dim]);

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
