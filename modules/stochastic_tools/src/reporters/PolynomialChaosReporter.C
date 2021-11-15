//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialChaosReporter.h"

#include "SobolReporter.h"
#include "StochasticReporter.h"

registerMooseObject("StochasticToolsApp", PolynomialChaosReporter);

InputParameters
PolynomialChaosReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Tool for extracting data from PolynomialChaos surrogates and computing statistics.");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "pc_name", "Name(s) of PolynomialChaos surrogate object(s).");
  params.addParam<bool>("include_data",
                        false,
                        "True to output information on the polynomial chaos model, including "
                        "polynomial types, orders, and coefficients.");
  MultiMooseEnum stats("mean=1 stddev=2 skewness=3 kurtosis=4", "");
  params.addParam<MultiMooseEnum>("statistics", stats, "Statistics to compute.");
  params.addParam<bool>("include_sobol", false, "True to compute Sobol indices.");
  params.addParam<std::vector<std::vector<Real>>>(
      "local_sensitivity_points",
      std::vector<std::vector<Real>>(0),
      "Points for each polynomial chaos surrogate specifying desired location of sensitivity "
      "measurement.");
  params.addParam<std::vector<SamplerName>>(
      "local_sensitivity_sampler",
      std::vector<SamplerName>(0),
      "Sampler for each polynomial chaos surrogate specifying desired location of sensitivity "
      "measurement.");
  return params;
}

PolynomialChaosReporter::PolynomialChaosReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _loc_point(getParam<std::vector<std::vector<Real>>>("local_sensitivity_points"))
{
  for (const auto & sn : getParam<std::vector<SamplerName>>("local_sensitivity_sampler"))
    _loc_sampler.push_back(&getSamplerByName(sn));

  for (const auto & nm : getParam<std::vector<UserObjectName>>("pc_name"))
  {
    // Gather polynomial chaos surrogates
    _pc.push_back(&getSurrogateModelByName<PolynomialChaos>(nm));
    // PolynomialChaos data reporter
    if (getParam<bool>("include_data"))
    {
      auto & pc_ptr = declareValueByName<const PolynomialChaos *>(nm, REPORTER_MODE_ROOT);
      pc_ptr = _pc.back();
    }
    // Statistics reporter
    for (const auto & stat : getParam<MultiMooseEnum>("statistics"))
      declareValueByName<std::pair<Real, std::vector<Real>>, PCStatisticsContext<Real>>(
          nm + "_" + stat.name(), REPORTER_MODE_ROOT, *_pc.back(), stat);
    // Sobol reporter
    if (getParam<bool>("include_sobol"))
      declareValueByName<std::pair<std::vector<Real>, std::vector<std::vector<Real>>>,
                         PCSobolContext<Real>>(nm + "_SOBOL", REPORTER_MODE_ROOT, *_pc.back());
    // Local sensitivity points reporter
    if (_loc_point.size() > 0)
    {
      if (_loc_point.size() < _pc.size())
        paramError("local_sensitivity_points",
                   "There must be a set of points for each inputted polynomial chaos model.");
      _loc_point_sense.push_back(&declareValueByName<std::vector<std::vector<Real>>>(
          nm + "_POINT_SENSITIVITY", REPORTER_MODE_ROOT));
    }
    // Local sensitivity sampler reporter
    if (_loc_sampler.size() > 0)
    {
      if (_loc_sampler.size() < _pc.size())
        paramError("local_sensitivity_sampler",
                   "There must be a sampler for each inputted polynomial chaos model.");
      _loc_sampler_sense.push_back(
          &declareValueByName<std::vector<std::vector<Real>>,
                              StochasticReporterContext<std::vector<Real>>>(
              nm + "_SAMPLE_SENSITIVITY", REPORTER_MODE_ROOT, *_loc_sampler[_pc.size() - 1]));
    }
  }
}

void
PolynomialChaosReporter::execute()
{
  if ((_loc_point.size() + _loc_sampler.size()) == 0)
    return;

  for (const auto & i : index_range(_pc))
  {
    const auto & pc = *_pc[i];
    const auto nparam = pc.getNumberOfParameters();
    if (_loc_point.size() > 0)
    {
      if (_loc_point[i].size() % pc.getNumberOfParameters() != 0)
        paramError("local_sensitivity_points",
                   "Number of values must be divisible by number of parameters in "
                   "Polynomial Chaos model.");

      const auto npoint = _loc_point[i].size() / nparam;
      _loc_point_sense[i]->resize(npoint);
      for (const auto & p : make_range(npoint))
      {
        const std::vector<Real> data(_loc_point[i].begin() + p * nparam,
                                     _loc_point[i].begin() + (p + 1) * nparam);
        (*_loc_point_sense[i])[p] = computeLocalSensitivity(pc, data);
      }

      if (_loc_sampler.size() > 0)
      {
        if (_loc_sampler[i]->getNumberOfCols() != nparam)
          paramError(
              "local_sensitivity_sampler",
              "Sampler ",
              _loc_sampler[i]->name(),
              " does not have the same number of columns as the number of dimensions in model ",
              pc.name(),
              ".");
        for (const auto & p : make_range(_loc_sampler[i]->getNumberOfLocalRows()))
          (*_loc_sampler_sense[i])[p] =
              computeLocalSensitivity(pc, _loc_sampler[i]->getNextLocalRow());
      }
    }
  }
}

std::vector<Real>
PolynomialChaosReporter::computeLocalSensitivity(const PolynomialChaos & pc,
                                                 const std::vector<Real> & data)
{
  std::vector<Real> sense(data.size());
  const auto val = pc.evaluate(data);
  for (const auto & d : index_range(data))
    sense[d] = data[d] / val * pc.computeDerivative(d, data);
  return sense;
}

void
to_json(nlohmann::json & json, const PolynomialChaos * const & pc)
{
  pc->store(json);
}

template <typename OutType>
PCStatisticsContext<OutType>::PCStatisticsContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
    const PolynomialChaos & pc,
    const MooseEnumItem & stat)
  : ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>(other, producer, state),
    _pc(pc),
    _stat(stat)
{
}

template <typename OutType>
void
PCStatisticsContext<OutType>::finalize()
{
  // Compute standard deviation
  OutType sig = OutType();
  if (_stat == "stddev" || _stat == "skewness" || _stat == "kurtosis")
    sig = _pc.computeStandardDeviation();

  OutType & val = this->_state.value().first;
  val = OutType();
  // Mean
  if (_stat == "mean" && this->processor_id() == 0)
    val = _pc.computeMean();
  // Standard Deviation
  else if (_stat == "stddev" && this->processor_id() == 0)
    val = sig;
  // Skewness
  else if (_stat == "skewness")
    val = _pc.powerExpectation(3) / (sig * sig * sig);
  // Kurtosis
  else if (_stat == "kurtosis")
    val = _pc.powerExpectation(4) / (sig * sig * sig * sig);
  this->_communicator.sum(val);

  ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>::finalize();
}

template <typename OutType>
void
PCStatisticsContext<OutType>::storeInfo(nlohmann::json & json) const
{
  ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>::storeInfo(json);
  json["stat"] = _stat.name();
}

template <typename OutType>
PCSobolContext<OutType>::PCSobolContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>>> & state,
    const PolynomialChaos & pc)
  : ReporterGeneralContext<std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>>>(
        other, producer, state),
    _pc(pc)
{
}

template <typename OutType>
void
PCSobolContext<OutType>::finalize()
{
  const unsigned int nparam = _pc.getNumberOfParameters();
  std::vector<OutType> & val = this->_state.value().first;
  val.clear();

  // Compute variance
  auto var = _pc.computeStandardDeviation();
  var *= var;

  // First order
  for (const auto & i : make_range(nparam))
    val.push_back(_pc.computeSobolIndex({i}) / var);
  // Total
  for (const auto & i : make_range(nparam))
    val.push_back(_pc.computeSobolTotal(i) / var);
  // Second order
  for (const auto & i : make_range(nparam))
    for (const auto & j : make_range(i + 1, nparam))
      val.push_back(_pc.computeSobolIndex({i, j}) / var);
}

template <typename OutType>
void
PCSobolContext<OutType>::store(nlohmann::json & json) const
{
  SobolReporterContext<std::vector<OutType>, OutType>::storeSobol(
      json, this->_state.value(), _pc.getNumberOfParameters());
}

template class PCStatisticsContext<Real>;
template class PCSobolContext<Real>;
