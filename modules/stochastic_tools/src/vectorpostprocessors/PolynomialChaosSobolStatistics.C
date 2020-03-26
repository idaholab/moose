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

  params.addRequiredParam<UserObjectName>("pc_name", "Name of PolynomialChaos.");

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
    _pc_uo(getSurrogateModel<PolynomialChaos>("pc_name")),
    _order(getParam<MultiMooseEnum>("sensitivity_order")),
    _stats(declareVector("value")),
    _total_ind(_order.contains("total") ? &declareVector("i_T") : nullptr)
{
}

void
PolynomialChaosSobolStatistics::initialSetup()
{
  _nind = 0;
  _nval = 0;

  if (_order.contains("all"))
  {
    _nind = _pc_uo.getNumberOfParameters();
    _nval = 1;
    for (unsigned int j = 0; j < _pc_uo.getNumberOfParameters(); ++j)
      _nval *= 2;
    _nval -= 1;

    for (unsigned int i = 1; i <= _nind; ++i)
    {
      std::set<std::set<unsigned int>> indi =
          buildSobolIndices(i, _pc_uo.getNumberOfParameters(), 0);
      _ind.insert(indi.begin(), indi.end());
    }
  }
  else
  {
    if (_order.contains("first"))
    {
      _nind = 1;
      _nval = _pc_uo.getNumberOfParameters();
      std::set<std::set<unsigned int>> indi =
          buildSobolIndices(_nind, _pc_uo.getNumberOfParameters(), 0);
      _ind.insert(indi.begin(), indi.end());
    }
    if (_order.contains("second"))
    {
      _nind = 2;
      _nval += Utility::binomial(_pc_uo.getNumberOfParameters(), static_cast<std::size_t>(2));
      std::set<std::set<unsigned int>> indi =
          buildSobolIndices(_nind, _pc_uo.getNumberOfParameters(), 0);
      _ind.insert(indi.begin(), indi.end());
    }
  }

  _ind_vector.reserve(_nind);
  for (unsigned int i = 0; i < _nind; ++i)
    _ind_vector.push_back(&declareVector("i_" + std::to_string(i + 1)));

  if (_order.contains("total"))
    _nval += _pc_uo.getNumberOfParameters();
}

void
PolynomialChaosSobolStatistics::initialize()
{
  _stats.reserve(_nval);

  if (_total_ind)
    _total_ind->reserve(_nval);

  for (unsigned int i = 0; i < _ind_vector.size(); ++i)
    _ind_vector[i]->reserve(_nval);
}

void
PolynomialChaosSobolStatistics::execute()
{
  // Compute standard deviation
  Real sig = _pc_uo.computeStandardDeviation();
  Real var = sig * sig;

  if (_order.contains("total"))
  {
    for (unsigned int d = 0; d < _pc_uo.getNumberOfParameters(); ++d)
    {
      _stats.push_back(_pc_uo.computeSobolTotal(d) / var);
      _total_ind->push_back(d);
      for (unsigned int i = 0; i < _ind_vector.size(); ++i)
        _ind_vector[i]->push_back(0);
    }
  }

  for (const auto & it : _ind)
  {
    _stats.push_back(_pc_uo.computeSobolIndex(it) / var);

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
  gatherSum(_stats);
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
