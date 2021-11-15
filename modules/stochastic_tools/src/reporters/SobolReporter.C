//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SobolReporter.h"
#include "SobolSampler.h"
#include "SobolCalculators.h"
#include "BootstrapCalculators.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("StochasticToolsApp", SobolReporter);

InputParameters
SobolReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Compute SOBOL statistics values of a given VectorPostprocessor or "
                             "Reporter objects and vectors.");
  params.addParam<SamplerName>("sampler", "SobolSampler object.");

  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");

  params.addParam<std::vector<ReporterName>>(
      "reporters", "List of Reporter values to utilized for statistic computations.");

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

SobolReporter::SobolReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _sobol_sampler(getSampler<SobolSampler>("sampler")),
    _ci_levels(getParam<std::vector<Real>>("ci_levels")),
    _ci_replicates(getParam<unsigned int>("ci_replicates")),
    _ci_seed(getParam<unsigned int>("ci_seed")),
    _initialized(false)
{
  // CI levels error checking
  if (!_ci_levels.empty())
  {
    if (*std::min_element(_ci_levels.begin(), _ci_levels.end()) <= 0)
      paramError("ci_levels", "The supplied levels must be greater than zero.");
    else if (*std::max_element(_ci_levels.begin(), _ci_levels.end()) >= 1)
      paramError("ci_levels", "The supplied levels must be less than 1.0");
  }

  if ((!isParamValid("reporters") && !isParamValid("vectorpostprocessors")) ||
      (getParam<std::vector<ReporterName>>("reporters").empty() &&
       getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors").empty()))
    mooseError(
        "The 'vectorpostprocessors' and/or 'reporters' parameters must be defined and non-empty.");
}

void
SobolReporter::initialize()
{
  if (_initialized)
    return;

  // Stats for Reporters
  if (isParamValid("reporters"))
  {
    std::vector<std::string> unsupported_types;
    const auto & reporter_names = getParam<std::vector<ReporterName>>("reporters");
    for (const auto & r_name : reporter_names)
    {
      if (hasReporterValueByName<std::vector<Real>>(r_name))
        declareValueHelper<Real>(r_name);
      else if (hasReporterValueByName<std::vector<std::vector<Real>>>(r_name))
        declareValueHelper<std::vector<Real>>(r_name);
      else
        unsupported_types.emplace_back(r_name.getCombinedName());
    }

    if (!unsupported_types.empty())
      paramError("reporters",
                 "The following reporter value(s) do not have a type supported by the "
                 "StatisticsReporter:\n",
                 MooseUtils::join(unsupported_types, ", "));
  }

  // Stats for VPP
  if (isParamValid("vectorpostprocessors"))
  {
    const auto & vpp_names = getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors");
    for (const auto & vpp_name : vpp_names)
    {
      const VectorPostprocessor & vpp_object =
          _fe_problem.getVectorPostprocessorObjectByName(vpp_name);
      const std::set<std::string> & vpp_vectors = vpp_object.getVectorNames();
      for (const auto & vec_name : vpp_vectors)
      {
        ReporterName r_name(vpp_name, vec_name);
        declareValueHelper<Real>(r_name);
      }
    }
  }

  _initialized = true;
}

void
SobolReporter::store(nlohmann::json & json) const
{
  Reporter::store(json);
  if (!_ci_levels.empty())
    json["confidence_intervals"] = {{"method", "percentile"},
                                    {"levels", _ci_levels},
                                    {"replicates", _ci_replicates},
                                    {"seed", _ci_seed}};

  json["num_params"] = _sobol_sampler.getNumberOfCols();
  json["indices"].push_back("FIRST_ORDER");
  json["indices"].push_back("TOTAL");
  if (_sobol_sampler.resample())
    json["indices"].push_back("SECOND_ORDER");
}

template <typename T>
void
SobolReporter::declareValueHelper(const ReporterName & r_name)
{
  const auto & mode = _fe_problem.getReporterData().getReporterMode(r_name);
  const auto & data = getReporterValueByName<std::vector<T>>(r_name);
  const std::string s_name = r_name.getObjectName() + "_" + r_name.getValueName();
  if (!_ci_levels.empty())
    declareValueByName<SobolState<T>, SobolReporterContext<std::vector<T>, T>>(
        s_name,
        REPORTER_MODE_ROOT,
        data,
        mode,
        _sobol_sampler,
        MooseEnum("percentile", "percentile"),
        _ci_levels,
        _ci_replicates,
        _ci_seed);
  else
    declareValueByName<SobolState<T>, SobolReporterContext<std::vector<T>, T>>(
        s_name, REPORTER_MODE_ROOT, data, mode, _sobol_sampler);
}

template <typename InType, typename OutType>
SobolReporterContext<InType, OutType>::SobolReporterContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<SobolState<OutType>> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const SobolSampler & sampler)
  : ReporterGeneralContext<SobolState<OutType>>(other, producer, state),
    _data(data),
    _data_mode(mode),
    _sampler(sampler),
    _calc(other, "SOBOL", _sampler.resample())
{
}

template <typename InType, typename OutType>
SobolReporterContext<InType, OutType>::SobolReporterContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<SobolState<OutType>> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const SobolSampler & sampler,
    const MooseEnum & ci_method,
    const std::vector<Real> & ci_levels,
    unsigned int ci_replicates,
    unsigned int ci_seed)
  : SobolReporterContext<InType, OutType>(other, producer, state, data, mode, sampler)
{
  _ci_calc_ptr =
      StochasticTools::makeBootstrapCalculator<std::vector<InType>, std::vector<OutType>>(
          ci_method, other, ci_levels, ci_replicates, ci_seed, _calc);
}

template <typename InType, typename OutType>
void
SobolReporterContext<InType, OutType>::finalize()
{
  auto & val = this->_state.value(); // Convenience
  val.first.clear();
  val.second.clear();

  const bool is_dist = _data_mode == REPORTER_MODE_DISTRIBUTED;
  if (is_dist || this->processor_id() == 0)
  {
    const std::size_t ncol =
        _sampler.resample() ? 2 * _sampler.getNumberOfCols() + 2 : _sampler.getNumberOfCols() + 2;
    const std::vector<InType> data_reshape =
        StochasticTools::reshapeVector(_data, ncol, /*row_major =*/true);

    val.first = _calc.compute(data_reshape, is_dist);
    if (_ci_calc_ptr)
      val.second = _ci_calc_ptr->compute(data_reshape, is_dist);
  }

  ReporterGeneralContext<SobolState<OutType>>::finalize();
}

template <typename InType, typename OutType>
void
SobolReporterContext<InType, OutType>::store(nlohmann::json & json) const
{
  storeSobol(json, this->_state.value(), _sampler.getNumberOfCols());
}

template <typename InType, typename OutType>
void
SobolReporterContext<InType, OutType>::storeSobol(nlohmann::json & json,
                                                  const SobolState<OutType> & val,
                                                  unsigned int nparam)
{
  if (val.first.empty())
    return;

  // Convenience
  const unsigned int nlevels = val.second.size();

  std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>> first_order;
  std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>> total;

  first_order.first.assign(val.first.begin(), val.first.begin() + nparam);
  total.first.assign(val.first.begin() + nparam, val.first.begin() + nparam + nparam);
  if (nlevels > 0)
  {
    first_order.second.resize(nlevels);
    total.second.resize(nlevels);
    for (const auto & l : make_range(nlevels))
    {
      first_order.second[l].assign(val.second[l].begin(), val.second[l].begin() + nparam);
      total.second[l].assign(val.second[l].begin() + nparam,
                             val.second[l].begin() + nparam + nparam);
    }
  }
  json["FIRST_ORDER"] = first_order;
  json["TOTAL"] = total;

  if (val.first.size() >= 2 * nparam)
  {
    std::pair<std::vector<std::vector<OutType>>, std::vector<std::vector<std::vector<OutType>>>>
        second_order;

    second_order.first.resize(nparam, std::vector<OutType>(nparam));
    for (const auto & i : make_range(nparam))
      second_order.first[i][i] = val.first[i];
    unsigned int ind = 2 * nparam;
    for (const auto & i : make_range(nparam))
      for (const auto & j : make_range(i + 1, nparam))
      {
        second_order.first[i][j] = val.first[ind++];
        second_order.first[j][i] = second_order.first[i][j];
      }

    if (nlevels > 0)
    {
      second_order.second.resize(nlevels, second_order.first);

      for (const auto & l : make_range(nlevels))
      {
        for (const auto & i : make_range(nparam))
          second_order.second[l][i][i] = val.second[l][i];
        ind = 2 * nparam;
        for (const auto & i : make_range(nparam))
          for (const auto & j : make_range(i + 1, nparam))
          {
            second_order.second[l][i][j] = val.second[l][ind++];
            second_order.second[l][j][i] = second_order.second[l][i][j];
          }
      }
    }
    json["SECOND_ORDER"] = second_order;
  }
}

template void SobolReporter::declareValueHelper<Real>(const ReporterName & r_name);
template class SobolReporterContext<std::vector<Real>, Real>;
template void SobolReporter::declareValueHelper<std::vector<Real>>(const ReporterName & r_name);
template class SobolReporterContext<std::vector<std::vector<Real>>, std::vector<Real>>;
