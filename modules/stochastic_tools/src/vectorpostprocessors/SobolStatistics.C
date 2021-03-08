//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SobolStatistics.h"
#include "SobolSampler.h"
#include "SobolCalculators.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("StochasticToolsApp", SobolStatistics);

InputParameters
SobolStatistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Compute SOBOL statistics values of a given VectorPostprocessor objects and vectors.");
  params.addParam<SamplerName>("sampler", "SobolSampler object.");
  params.addParam<VectorPostprocessorName>(
      "results", "StochasticResults object containing data to use for calculation.");
  return params;
}

SobolStatistics::SobolStatistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), _sobol_sampler(getSampler<SobolSampler>("sampler"))
{
}

void
SobolStatistics::initialSetup()
{
  const VectorPostprocessorName & vpp_name = getParam<VectorPostprocessorName>("results");
  const VectorPostprocessor & vpp_object = _fe_problem.getVectorPostprocessorObjectByName(vpp_name);
  const std::set<std::string> & vpp_vectors = vpp_object.getVectorNames();
  for (const auto & vec_name : vpp_vectors)
  {
    _result_vectors.push_back(std::make_pair(&getVectorPostprocessorValueByName(vpp_name, vec_name),
                                             vpp_object.isDistributed()));
    _sobol_stat_vectors.push_back(&declareVector(vpp_name + "_" + vec_name));
  }
}

void
SobolStatistics::execute()
{
  TIME_SECTION("execute", 3, "Executing Sobol Statistics");

  StochasticTools::SobolCalculator calc(
      *this, _sobol_sampler.getNumberOfCols(), _sobol_sampler.resample());
  for (std::size_t i = 0; i < _result_vectors.size(); ++i)
    (*_sobol_stat_vectors[i]) =
        calc.compute(*(_result_vectors[i].first), _result_vectors[i].second);
}
