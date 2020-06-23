//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PolynomialChaosStatistics.h"

registerMooseObject("StochasticToolsApp", PolynomialChaosStatistics);

InputParameters
PolynomialChaosStatistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tool for calculating statistics with polynomial chaos expansion.");

  params.addRequiredParam<std::vector<UserObjectName>>("pc_name", "Name of PolynomialChaos.");

  MultiMooseEnum stats("mean=1 stddev=2 skewness=3 kurtosis=4");
  params.addRequiredParam<MultiMooseEnum>("compute", stats, "Type of statistic to output.");
  return params;
}

PolynomialChaosStatistics::PolynomialChaosStatistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SurrogateModelInterface(this),
    _type(getParam<MultiMooseEnum>("compute")),
    _stat_type(declareVector("type"))
{
  for (const auto & item : _type)
    _stat_type.push_back(item.id());

  const auto & pc_names = getParam<std::vector<UserObjectName>>("pc_name");
  _pc_uo.reserve(pc_names.size());
  _stats.reserve(pc_names.size());
  for (const auto & nm : pc_names)
  {
    _pc_uo.push_back(&getSurrogateModelByName<PolynomialChaos>(nm));
    _stats.push_back(&declareVector(nm));
    _stats.back()->reserve(_stat_type.size());
  }
}

void
PolynomialChaosStatistics::execute()
{
  for (unsigned int i = 0; i < _stats.size(); ++i)
  {
    // Compute standard deviation
    Real sig = 0.0;
    if (_type.contains("stddev") || _type.contains("skewness") || _type.contains("kurtosis"))
      sig = _pc_uo[i]->computeStandardDeviation();

    for (const auto & item : _type)
    {
      Real val = 0.0;
      // Mean
      if (item == "mean" && processor_id() == 0)
        val = _pc_uo[i]->computeMean();
      // Standard Deviation
      else if (item == "stddev" && processor_id() == 0)
        val = sig;
      // Skewness
      else if (item == "skewness")
        val = _pc_uo[i]->powerExpectation(3) / (sig * sig * sig);
      // Kurtosis
      else if (item == "kurtosis")
        val = _pc_uo[i]->powerExpectation(4) / (sig * sig * sig * sig);
      _stats[i]->push_back(val);
    }
  }
}

void
PolynomialChaosStatistics::finalize()
{
  for (auto & it : _stats)
    gatherSum(*it);
}
