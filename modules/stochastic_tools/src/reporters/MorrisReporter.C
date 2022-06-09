//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MorrisReporter.h"

#include "MorrisSampler.h"

registerMooseObject("StochasticToolsApp", MorrisReporter);

InputParameters
MorrisReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Compute global sensitivities using the Morris method.");

  params.addRequiredParam<SamplerName>("sampler", "Morris sampler used to generate samples.");
  params.addParam<std::vector<VectorPostprocessorName>>(
      "vectorpostprocessors",
      "List of VectorPostprocessor(s) to utilized for statistic computations.");
  params.addParam<std::vector<ReporterName>>(
      "reporters", "List of Reporter values to utilized for statistic computations.");

  // Confidence Levels
  params.addParam<std::vector<Real>>(
      "ci_levels",
      std::vector<Real>(),
      "A vector of confidence levels to consider, values must be in (0, 1).");
  params.addParam<unsigned int>("ci_replicates",
                                10000,
                                "The number of replicates to use when computing confidence level "
                                "intervals. This is basically the number of times the statistics "
                                "are recomputed with a random selection of indices.");
  params.addParam<unsigned int>("ci_seed",
                                1,
                                "The random number generator seed used for creating replicates "
                                "while computing confidence level intervals.");
  return params;
}

MorrisReporter::MorrisReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _sampler(getSampler("sampler")),
    _ci_levels(getParam<std::vector<Real>>("ci_levels")),
    _ci_replicates(getParam<unsigned int>("ci_replicates")),
    _ci_seed(getParam<unsigned int>("ci_seed")),
    _initialized(false)
{
  if (!dynamic_cast<MorrisSampler *>(&_sampler))
    paramError("sampler", "Computing Morris sensitivities requires the use of a Morris sampler.");

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
MorrisReporter::initialize()
{
  if (_initialized)
    return;

  // Stats for Reporters
  if (isParamValid("reporters"))
  {
    std::vector<std::string> unsupported_types;
    for (const auto & r_name : getParam<std::vector<ReporterName>>("reporters"))
    {
      if (hasReporterValueByName<std::vector<Real>>(r_name))
        declareValueHelper<Real>(r_name);
      else if (hasReporterValueByName<std::vector<std::vector<Real>>>(r_name))
        declareValueHelper<std::vector<Real>>(r_name);
      else
        unsupported_types.push_back(r_name.getCombinedName());
    }

    if (!unsupported_types.empty())
      paramError("reporters",
                 "The following reporter value(s) do not have a type supported by the "
                 "MorrisReporter:\n",
                 MooseUtils::join(unsupported_types, ", "));
  }

  // Stats for VPP
  if (isParamValid("vectorpostprocessors"))
    for (const auto & vpp_name :
         getParam<std::vector<VectorPostprocessorName>>("vectorpostprocessors"))
      for (const auto & vec_name :
           _fe_problem.getVectorPostprocessorObjectByName(vpp_name).getVectorNames())
        declareValueHelper<Real>(ReporterName(vpp_name, vec_name));
  _initialized = true;
}

void
MorrisReporter::store(nlohmann::json & json) const
{
  Reporter::store(json);
  if (!_ci_levels.empty())
    json["confidence_intervals"] = {{"method", "percentile"},
                                    {"levels", _ci_levels},
                                    {"replicates", _ci_replicates},
                                    {"seed", _ci_seed}};
  json["num_params"] = _sampler.getNumberOfCols();
  json["num_trajectories"] = _sampler.getNumberOfRows() / (_sampler.getNumberOfCols() + 1);
  if (_sampler.parameters().isParamValid("levels"))
    json["levels"] = _sampler.parameters().get<unsigned int>("levels");
}

template <typename DataType>
void
MorrisReporter::declareValueHelper(const ReporterName & r_name)
{
  const auto & data = getReporterValueByName<std::vector<DataType>>(r_name);
  const std::string s_name = r_name.getObjectName() + "_" + r_name.getValueName();
  if (!_ci_levels.empty())
    declareValueByName<MorrisState<DataType>, MorrisReporterContext<DataType>>(
        s_name,
        REPORTER_MODE_ROOT,
        _sampler,
        data,
        MooseEnum("percentile", "percentile"),
        _ci_levels,
        _ci_replicates,
        _ci_seed);
  else
    declareValueByName<MorrisState<DataType>, MorrisReporterContext<DataType>>(
        s_name, REPORTER_MODE_ROOT, _sampler, data);
}

template <typename DataType>
MorrisReporterContext<DataType>::MorrisReporterContext(const libMesh::ParallelObject & other,
                                                       const MooseObject & producer,
                                                       ReporterState<MorrisState<DataType>> & state,
                                                       Sampler & sampler,
                                                       const std::vector<DataType> & data)
  : ReporterGeneralContext<MorrisState<DataType>>(other, producer, state),
    _sampler(sampler),
    _data(data)
{
  MultiMooseEnum items("mean meanabs stddev", "mean meanabs stddev", true);
  _mu_calc = StochasticTools::makeCalculator<std::vector<DataType>, DataType>(items[0], other);
  _mustar_calc = StochasticTools::makeCalculator<std::vector<DataType>, DataType>(items[1], other);
  _sig_calc = StochasticTools::makeCalculator<std::vector<DataType>, DataType>(items[2], other);

  // Initialize state
  auto & mu = this->_state.value()["mu"].first;
  mu.resize(_sampler.getNumberOfCols());
  auto & mu_star = this->_state.value()["mu_star"].first;
  mu_star.resize(_sampler.getNumberOfCols());
  auto & sig = this->_state.value()["sigma"].first;
  sig.resize(_sampler.getNumberOfCols());
}

template <typename DataType>
MorrisReporterContext<DataType>::MorrisReporterContext(const libMesh::ParallelObject & other,
                                                       const MooseObject & producer,
                                                       ReporterState<MorrisState<DataType>> & state,
                                                       Sampler & sampler,
                                                       const std::vector<DataType> & data,
                                                       const MooseEnum & ci_method,
                                                       const std::vector<Real> & ci_levels,
                                                       unsigned int ci_replicates,
                                                       unsigned int ci_seed)
  : MorrisReporterContext<DataType>(other, producer, state, sampler, data)
{
  _ci_mu_calc = StochasticTools::makeBootstrapCalculator<std::vector<DataType>, DataType>(
      ci_method, other, ci_levels, ci_replicates, ci_seed, *_mu_calc);
  _ci_mustar_calc = StochasticTools::makeBootstrapCalculator<std::vector<DataType>, DataType>(
      ci_method, other, ci_levels, ci_replicates, ci_seed, *_mustar_calc);
  _ci_sig_calc = StochasticTools::makeBootstrapCalculator<std::vector<DataType>, DataType>(
      ci_method, other, ci_levels, ci_replicates, ci_seed, *_sig_calc);

  // Initialize state
  auto & mu = this->_state.value()["mu"].second;
  mu.resize(_sampler.getNumberOfCols());
  auto & mu_star = this->_state.value()["mu_star"].second;
  mu_star.resize(_sampler.getNumberOfCols());
  auto & sig = this->_state.value()["sigma"].second;
  sig.resize(_sampler.getNumberOfCols());
}

template <typename DataType>
void
MorrisReporterContext<DataType>::finalize()
{
  dof_id_type offset;
  if (_data.size() == _sampler.getNumberOfRows())
    offset = _sampler.getLocalRowBegin();
  else if (_data.size() == _sampler.getNumberOfLocalRows())
    offset = 0;
  else
    mooseError("Data size inconsistency. Expected data vector to have ",
               _sampler.getNumberOfLocalRows(),
               " (local) or ",
               _sampler.getNumberOfRows(),
               " (global) elements, but actually has ",
               _data.size(),
               " elements. Are you using the same sampler?");

  const dof_id_type nc = _sampler.getNumberOfCols(); // convenience
  if (_sampler.getNumberOfLocalRows() % (nc + 1) > 0)
    mooseError(
        "Sampler does not have an appropriate number of rows. Are you using a Morris sampler?");

  std::vector<std::vector<DataType>> elem_effects(
      nc, std::vector<DataType>(_sampler.getNumberOfLocalRows() / (nc + 1)));
  RealEigenMatrix x(nc + 1, nc);
  std::vector<DataType> y(nc + 1);

  for (dof_id_type p = 0; p < _sampler.getNumberOfLocalRows(); ++p)
  {
    dof_id_type traj = p / (nc + 1);
    dof_id_type tind = p % (nc + 1);
    const std::vector<Real> row = _sampler.getNextLocalRow();
    for (unsigned int k = 0; k < nc; ++k)
      x(tind, k) = row[k];
    y[tind] = _data[p + offset];

    if (tind == nc)
    {
      const std::vector<DataType> ee = computeElementaryEffects(x, y);
      for (unsigned int k = 0; k < nc; ++k)
        elem_effects[k][traj] = ee[k];
    }
  }

  auto & mu = this->_state.value()["mu"];
  auto & mustar = this->_state.value()["mu_star"];
  auto & sig = this->_state.value()["sigma"];
  for (unsigned int k = 0; k < nc; ++k)
  {
    mu.first[k] = _mu_calc->compute(elem_effects[k], true);
    mustar.first[k] = _mustar_calc->compute(elem_effects[k], true);
    sig.first[k] = _sig_calc->compute(elem_effects[k], true);

    if (_ci_mu_calc)
      mu.second[k] = _ci_mu_calc->compute(elem_effects[k], true);
    if (_ci_mustar_calc)
      mustar.second[k] = _ci_mustar_calc->compute(elem_effects[k], true);
    if (_ci_sig_calc)
      sig.second[k] = _ci_sig_calc->compute(elem_effects[k], true);
  }
}

template <>
std::vector<Real>
MorrisReporterContext<Real>::computeElementaryEffects(const RealEigenMatrix & x,
                                                      const std::vector<Real> & y) const
{
  const auto k = y.size() - 1;
  const RealEigenMatrix dx = x.bottomRows(k) - x.topRows(k);
  RealEigenVector dy(k);
  for (unsigned int j = 0; j < k; ++j)
    dy(j) = y[j + 1] - y[j];

  const RealEigenVector u = dx.fullPivLu().solve(dy);
  return std::vector<Real>(u.data(), u.data() + u.size());
}

template <>
std::vector<std::vector<Real>>
MorrisReporterContext<std::vector<Real>>::computeElementaryEffects(
    const RealEigenMatrix & x, const std::vector<std::vector<Real>> & y) const
{
  const auto k = y.size() - 1;
  const RealEigenMatrix dx = x.bottomRows(k) - x.topRows(k);
  const auto solver = dx.fullPivLu();
  RealEigenVector dy(k);
  std::vector<std::vector<Real>> ee(k, std::vector<Real>(y[0].size()));
  for (unsigned int i = 0; i < y[0].size(); ++i)
  {
    for (unsigned int j = 0; j < k; ++j)
      dy(j) = y[j + 1][i] - y[j][i];
    const RealEigenVector u = solver.solve(dy);
    for (unsigned int j = 0; j < k; ++j)
      ee[j][i] = u(j);
  }
  return ee;
}

template void MorrisReporter::declareValueHelper<Real>(const ReporterName & r_name);
template class MorrisReporterContext<Real>;
template void MorrisReporter::declareValueHelper<std::vector<Real>>(const ReporterName & r_name);
template class MorrisReporterContext<std::vector<Real>>;
