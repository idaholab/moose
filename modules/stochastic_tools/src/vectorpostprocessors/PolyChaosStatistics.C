//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PolyChaosStatistics.h"

registerMooseObject("StochasticToolsApp", PolyChaosStatistics);

defineLegacyParams(PolyChaosStatistics);

InputParameters
PolyChaosStatistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tool for calculating statistics with polynomial chaos expansion.");

  params.addRequiredParam<UserObjectName>("pc_name", "Name of PolynomialChaos.");

  MultiMooseEnum stats("mean=1 stddev=2 skewness=3 kurtosis=4");
  params.addRequiredParam<MultiMooseEnum>("compute", stats, "Type of statistic to output.");
  return params;
}

PolyChaosStatistics::PolyChaosStatistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _pc_uo(getUserObject<PolynomialChaos>("pc_name")),
    _type(getParam<MultiMooseEnum>("compute")),
    _stat_type(declareVector("type")),
    _stats(declareVector("value"))
{
  for (const auto & item : _type)
    _stat_type.push_back(item.id());

  _stats.resize(_stat_type.size());
}

void
PolyChaosStatistics::execute()
{
  // Compute standard deviation
  Real sig = 0.0;
  if (_type.contains("stddev") || _type.contains("skewness") || _type.contains("kurtosis"))
    sig = _pc_uo.computeSTD();

  for (unsigned int t = 0; t < _type.size(); ++t)
  {
    // Mean
    if (_type.get(t) == 1 && processor_id() == 0)
      _stats[t] = _pc_uo.computeMean();

    // Standard Deviation
    if (_type.get(t) == 2 && processor_id() == 0)
      _stats[t] = sig;

    // Skewness
    if (_type.get(t) == 3)
      _stats[t] = _pc_uo.powerExpectation(3, true) / (sig * sig * sig);

    // Kurtosis
    if (_type.get(t) == 4)
      _stats[t] = _pc_uo.powerExpectation(4, true) / (sig * sig * sig * sig);
  }
}

void
PolyChaosStatistics::finalize()
{
  gatherSum(_stats);
}

void
PolyChaosStatistics::threadJoin(const UserObject & y)
{
  const PolyChaosStatistics & pps = static_cast<const PolyChaosStatistics &>(y);
  for (unsigned int i = 0; i < _stats.size(); ++i)
    _stats[i] += pps._stats[i];
}
