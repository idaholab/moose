//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveImportanceStats.h"
#include "Sampler.h"
#include "Distribution.h"
#include "Normal.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", AdaptiveImportanceStats);

InputParameters
AdaptiveImportanceStats::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter to compute statistics corresponding to the "
                             "AdaptiveImportanceSampler.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>("mu_imp", "mu_imp", "Means of the importance distributions.");
  params.addParam<ReporterValueName>(
      "std_imp", "std_imp", "Standard deviations of the importance distributions.");
  params.addParam<ReporterValueName>("pf", "pf", "Failure probability estimate.");
  params.addParam<ReporterValueName>(
      "cov_pf", "cov_pf", "Coefficient of variation of failure probability.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  return params;
}

AdaptiveImportanceStats::AdaptiveImportanceStats(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _mu_imp(declareValue<std::vector<Real>>("mu_imp")),
    _std_imp(declareValue<std::vector<Real>>("std_imp")),
    _pf(declareValue<std::vector<Real>>("pf")),
    _cov_pf(declareValue<std::vector<Real>>("cov_pf")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _ais(getSampler<AdaptiveImportanceSampler>("sampler")),
    _check_step(std::numeric_limits<int>::max())
{
  // Initialize variables
  const auto rows = _ais.getNumberOfRows();
  _mu_imp.resize(rows);
  _std_imp.resize(rows);
  _pf.resize(1);
  _cov_pf.resize(1);
  _pf_sum = 0.0;
  _var_sum = 0.0;
  _distributions_store = _ais.getDistributionNames();
  _factor = _ais.getStdFactor();
}

void
AdaptiveImportanceStats::execute()
{
  if (_ais.getNumberOfLocalRows() == 0 || _check_step == _step)
  {
    _check_step = _step;
    return;
  }

  // Compute AdaptiveImportanceSampler statistics at each sample during the evaluation phase only.
  if (_step > _ais.getNumSamplesTrain())
  {
    // Get the statistics of the importance distributions in the standard Normal space.
    _mu_imp = _ais.getImportanceVectorMean();
    _std_imp = _ais.getImportanceVectorStd();

    // Get the failure probability estimate.
    const Real tmp = _ais.getUseAbsoluteValue() ? std::abs(_output_value[0]) : _output_value[0];
    const bool output_limit_reached = tmp >= _ais.getOutputLimit();
    Real prod1 = output_limit_reached ? 1.0 : 0.0;
    std::vector<Real> input1 = _ais.getNextLocalRow();
    Real input_tmp = 0.0;
    for (dof_id_type ss = 0; ss < _ais.getNumberOfCols(); ++ss)
    {
      input_tmp = Normal::quantile(_distributions_store[ss]->cdf(input1[ss]), 0.0, 1.0);
      prod1 = prod1 * (Normal::pdf(input_tmp, 0.0, 1.0) /
                       Normal::pdf(input_tmp, _mu_imp[ss], _factor * _std_imp[ss]));
    }
    _pf_sum += prod1;
    _var_sum += Utility::pow<2>(prod1);
    _pf[0] = _pf_sum / (_step - _ais.getNumSamplesTrain());

    // Get coefficient of variation of failure probability.
    Real tmp_var =
        std::sqrt(1.0 / (_step - _ais.getNumSamplesTrain()) *
                  (_var_sum / (_step - _ais.getNumSamplesTrain()) - Utility::pow<2>(_pf[0])));
    _cov_pf[0] = tmp_var / _pf[0];
  }
}
