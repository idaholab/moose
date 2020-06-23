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
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    _sobol_sampler(getSampler<SobolSampler>("sampler")),
    _perf_execute(registerTimedSection("execute", 1))
{
}

void
SobolStatistics::initialSetup()
{
  const VectorPostprocessorName & vpp_name = getParam<VectorPostprocessorName>("results");
  const std::vector<std::pair<std::string, VectorPostprocessorData::VectorPostprocessorState>> &
      vpp_vectors = _fe_problem.getVectorPostprocessorVectors(vpp_name);
  for (const auto & the_pair : vpp_vectors)
  {
    _result_vectors.push_back(
        std::make_pair(&getVectorPostprocessorValueByName(vpp_name, the_pair.first),
                       the_pair.second.is_distributed));
    _sobol_stat_vectors.push_back(&declareVector(vpp_name + "_" + the_pair.first));
  }
}

void
SobolStatistics::execute()
{
  TIME_SECTION(_perf_execute);

  StochasticTools::SobolCalculator calc(
      *this, _sobol_sampler.getNumberOfCols(), _sobol_sampler.resample());
  for (std::size_t i = 0; i < _result_vectors.size(); ++i)
    (*_sobol_stat_vectors[i]) =
        calc.compute(*(_result_vectors[i].first), _result_vectors[i].second);
}
