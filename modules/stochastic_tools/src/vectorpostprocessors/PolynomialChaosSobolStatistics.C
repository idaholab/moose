//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PolynomialChaosSobolStatistics.h"

#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", PolynomialChaosSobolStatistics);

InputParameters
PolynomialChaosSobolStatistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription(
      "Compute SOBOL statistics values of a trained PolynomialChaos surrogate.");

  params.addRequiredParam<std::vector<UserObjectName>>("pc_name", "Name of PolynomialChaos.");

  MultiMooseEnum stats("first second all total");
  params.addRequiredParam<MultiMooseEnum>(
      "sensitivity_order",
      stats,
      "Order of sensitivities to compute. The all option computes all possible orders.");
  return params;
}

PolynomialChaosSobolStatistics::PolynomialChaosSobolStatistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SurrogateModelInterface(this),
    _order(getParam<MultiMooseEnum>("sensitivity_order"))
{
  const auto & pc_names = getParam<std::vector<UserObjectName>>("pc_name");
  _pc_uo.reserve(pc_names.size());
  _stats.reserve(pc_names.size());
  for (const auto & nm : pc_names)
  {
    _pc_uo.push_back(&getSurrogateModelByName<PolynomialChaos>(nm));
    _stats.push_back(&declareVector(nm));
  }

  if (_order.contains("total"))
    _total_ind = &declareVector("i_T");
}

void
PolynomialChaosSobolStatistics::initialSetup()
{
  _ndim = _pc_uo[0]->getNumberOfParameters();
  for (unsigned int m = 1; m < _pc_uo.size(); ++m)
    if (_pc_uo[m]->getNumberOfParameters() != _ndim)
      mooseError(
          "All inputted PolynomialChaos surrogates must have the same number of parameters.");

  _nind = 0;
  _nval = 0;

  if (_order.contains("all"))
  {
    _nind = _ndim;
    _nval = 1;
    for (unsigned int j = 0; j < _ndim; ++j)
      _nval *= 2;
    _nval -= 1;

    for (unsigned int i = 1; i <= _nind; ++i)
    {
      std::set<std::set<unsigned int>> indi = buildSobolIndices(i, _ndim, 0);
      _ind.insert(indi.begin(), indi.end());
    }
  }
  else
  {
    if (_order.contains("first"))
    {
      _nind = 1;
      _nval = _ndim;
      std::set<std::set<unsigned int>> indi = buildSobolIndices(_nind, _ndim, 0);
      _ind.insert(indi.begin(), indi.end());
    }
    if (_order.contains("second"))
    {
      _nind = 2;
      _nval += Utility::binomial(_ndim, static_cast<unsigned int>(2));
      std::set<std::set<unsigned int>> indi = buildSobolIndices(_nind, _ndim, 0);
      _ind.insert(indi.begin(), indi.end());
    }
  }

  _ind_vector.reserve(_nind);
  for (unsigned int i = 0; i < _nind; ++i)
    _ind_vector.push_back(&declareVector("i_" + std::to_string(i + 1)));

  if (_order.contains("total"))
    _nval += _ndim;
}

void
PolynomialChaosSobolStatistics::initialize()
{
  for (auto & vec : _stats)
    vec->reserve(_nval);

  if (_total_ind)
    _total_ind->reserve(_nval);

  for (unsigned int i = 0; i < _ind_vector.size(); ++i)
    _ind_vector[i]->reserve(_nval);
}

void
PolynomialChaosSobolStatistics::execute()
{
  // Compute standard deviation
  std::vector<Real> var(_pc_uo.size());
  for (unsigned int m = 0; m < _pc_uo.size(); ++m)
  {
    Real sig = _pc_uo[m]->computeStandardDeviation();
    var[m] = sig * sig;
  }

  if (_order.contains("total"))
  {
    for (unsigned int d = 0; d < _ndim; ++d)
    {
      for (unsigned int m = 0; m < _pc_uo.size(); ++m)
        _stats[m]->push_back(_pc_uo[m]->computeSobolTotal(d) / var[m]);
      _total_ind->push_back(d);
      for (unsigned int i = 0; i < _ind_vector.size(); ++i)
        _ind_vector[i]->push_back(0);
    }
  }

  for (const auto & it : _ind)
  {
    for (unsigned int m = 0; m < _pc_uo.size(); ++m)
      _stats[m]->push_back(_pc_uo[m]->computeSobolIndex(it) / var[m]);

    unsigned int i = 0;
    for (const auto & jt : it)
      _ind_vector[i++]->push_back(jt);
    for (unsigned int j = it.size(); j < _nind; ++j)
      _ind_vector[j]->push_back(*it.rbegin());

    if (_total_ind)
      _total_ind->push_back(0);
  }
}

void
PolynomialChaosSobolStatistics::finalize()
{
  for (auto & vec : _stats)
    gatherSum(*vec);
}

std::set<std::set<unsigned int>>
PolynomialChaosSobolStatistics::buildSobolIndices(const unsigned int nind,
                                                  const unsigned int ndim,
                                                  const unsigned int sdim)
{
  std::set<std::set<unsigned int>> ind;
  if (nind == 1)
  {
    for (unsigned int d = sdim; d < ndim; ++d)
      ind.insert({d});
  }
  else
  {
    for (unsigned int d = sdim; d < ndim; ++d)
    {
      std::set<std::set<unsigned int>> ind1 = buildSobolIndices(nind - 1, ndim, d + 1);
      std::set<std::set<unsigned int>> ind2;
      for (std::set<std::set<unsigned int>>::iterator it = ind1.begin(); it != ind1.end(); ++it)
      {
        std::set<unsigned int> copy = *it;
        copy.insert(d);
        ind2.insert(copy);
      }
      ind.insert(ind2.begin(), ind2.end());
    }
  }

  return ind;
}

bool
PolynomialChaosSobolStatistics::orderSobolIndices(const std::set<unsigned int> & a,
                                                  const std::set<unsigned int> & b)
{
  if (a.size() != b.size())
    return a.size() < b.size();
  else
    return a < b;
}
