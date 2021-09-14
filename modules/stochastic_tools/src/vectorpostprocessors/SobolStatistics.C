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
#include "BootstrapCalculators.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObjectDeprecated("StochasticToolsApp", SobolStatistics, "11/03/2021 12:00");

InputParameters
SobolStatistics::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Compute SOBOL statistics values of a given VectorPostprocessor objects and vectors.");
  params.addParam<SamplerName>("sampler", "SobolSampler object.");
  params.addParam<VectorPostprocessorName>(
      "results", "StochasticResults object containing data to use for calculation.");

  params.addParam<std::vector<Real>>(
      "ci_levels",
      std::vector<Real>(),
      "A vector of confidence levels to consider, values must be in (0, 1).");
  params.addParam<unsigned int>(
      "ci_replicates",
      10000,
      "The number of replicates to use when computing confidence level intervals.");
  params.addParam<unsigned int>("ci_seed",
                                1,
                                "The random number generator seed used for creating replicates "
                                "while computing confidence level intervals.");
  return params;
}

SobolStatistics::SobolStatistics(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _sobol_sampler(getSampler<SobolSampler>("sampler")),
    _ci_levels(getParam<std::vector<Real>>("ci_levels")),
    _ci_replicates(getParam<unsigned int>("ci_replicates")),
    _ci_seed(getParam<unsigned int>("ci_seed"))
{
}

void
SobolStatistics::initialSetup()
{
  const VectorPostprocessorName & vpp_name = getParam<VectorPostprocessorName>("results");
  const VectorPostprocessor & vpp_object = _fe_problem.getVectorPostprocessorObjectByName(vpp_name);
  const std::set<std::string> & vpp_vectors = vpp_object.getVectorNames();
  _sobol_ci_vectors.resize(_ci_levels.size());
  for (const auto & vec_name : vpp_vectors)
  {
    ReporterName rname(vpp_name, vec_name);
    _result_vectors.push_back(std::make_pair(
        &getReporterValueByName<VectorPostprocessorValue>(rname), vpp_object.isDistributed()));
    _sobol_stat_vectors.push_back(&declareVector(vpp_name + "_" + vec_name));

    for (const auto & l : index_range(_ci_levels))
    {
      std::stringstream vname; /// Vectors computed by this object
      vname << vpp_name << "_" << vec_name << "_" << _ci_levels[l] * 100 << "CI";
      _sobol_ci_vectors[l].push_back(&declareVector(vname.str()));
    }
  }
}

void
SobolStatistics::execute()
{
  TIME_SECTION("execute", 3, "Executing Sobol Statistics");

  StochasticTools::SobolCalculator<std::vector<Real>, Real> calc(
      *this, "SOBOL", _sobol_sampler.resample());
  auto boot_calc = _ci_levels.empty()
                       ? nullptr
                       : makeBootstrapCalculator(MooseEnum("percentile", "percentile"),
                                                 *this,
                                                 _ci_levels,
                                                 _ci_replicates,
                                                 _ci_seed,
                                                 calc);
  for (std::size_t i = 0; i < _result_vectors.size(); ++i)
  {
    const std::size_t ncol = _sobol_sampler.resample() ? 2 * _sobol_sampler.getNumberOfCols() + 2
                                                       : _sobol_sampler.getNumberOfCols() + 2;
    const std::vector<std::vector<Real>> data =
        StochasticTools::reshapeVector(*(_result_vectors[i].first), ncol, /*row_major =*/true);
    (*_sobol_stat_vectors[i]) = calc.compute(data, _result_vectors[i].second);

    if (boot_calc)
    {
      std::vector<std::vector<Real>> sobol_ci = boot_calc->compute(data, _result_vectors[i].second);
      for (const auto & l : index_range(sobol_ci))
        (*_sobol_ci_vectors[l][i]) = std::move(sobol_ci[l]);
    }
  }
}
