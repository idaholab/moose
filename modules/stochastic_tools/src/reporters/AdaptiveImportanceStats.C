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
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in Adaptive Monte Carlo type of algorithms.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>("mu_imp", "mu_imp", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("std_imp", "std_imp", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("pf", "pf", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("cov_pf", "cov_pf", "Uncertain inputs to the model.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  return params;
}

AdaptiveImportanceStats::AdaptiveImportanceStats(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _mu_imp(declareValue<std::vector<Real>>("mu_imp")),
    _std_imp(declareValue<std::vector<Real>>("std_imp")),
    _pf(declareValue<std::vector<double>>("pf")),
    _cov_pf(declareValue<std::vector<double>>("cov_pf")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _sampler(getSampler("sampler")),
    _ais(dynamic_cast<const AdaptiveImportanceSampler *>(&_sampler)),
    _check_step(std::numeric_limits<int>::max())
{
  if (!_ais)
    paramError("sampler", "The selected sampler is not of type AdaptiveImportance.");

  const auto rows = _sampler.getNumberOfRows();
  _mu_imp.resize(rows);
  _std_imp.resize(rows);
  _pf.resize(1);
  _cov_pf.resize(1);
  _pf_sum = 0.0;
  _var_sum = 0.0;

}

void
AdaptiveImportanceStats::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _step)
  {
    _check_step = _step;
    return;
  }

  if (_step > _ais->getNumSamplesTrain())
  {
    _mu_imp = _ais->getImportanceVectorMean();
    _std_imp = _ais->getImportanceVectorStd();

    const Real tmp = _ais->getUseAbsoluteValue() ? std::abs(_output_value[0]) : _output_value[0];
    const bool output_limit_reached = tmp >= _ais->getOutputLimit();
    Real prod1 = output_limit_reached ? 1.0 : 0.0;
    std::vector<Real> input1 = _sampler.getNextLocalRow();
    Real input_tmp = 0.0;
    std::vector<const Distribution *> _distributions1 = _ais->getDistributionNames();
    Real factor1 = _ais->getStdFactor();
    for (dof_id_type ss = 0; ss < _sampler.getNumberOfCols(); ++ss)
    {
      input_tmp = Normal::quantile(_distributions1[ss]->cdf(input1[ss]), 0.0, 1.0);
      prod1 = prod1 * (Normal::pdf(input_tmp, 0.0, 1.0) / Normal::pdf(input_tmp, _mu_imp[ss], factor1*_std_imp[ss]));
    }
    _pf_sum += prod1;
    _var_sum += std::pow(prod1, 2.0);
    _pf[0] = _pf_sum / (_step - _ais->getNumSamplesTrain());

    // Double precision is needed below.
    // std::cout << "Term 1 is " << _var_sum/(_step - _ais->getNumSamplesTrain()) << std::endl;
    // std::cout << "Term 2 is " << std::pow(_pf[0], 2) << std::endl;
    // std::cout << "Term is " << _var_sum/(_step - _ais->getNumSamplesTrain()) - std::pow(_pf[0], 2) << std::endl;
    // std::cout << std::setw(16) << "Term is " << std::pow(1/(_step - _ais->getNumSamplesTrain()) * (_var_sum/(_step - _ais->getNumSamplesTrain()) - std::pow(_pf[0], 2)), 0.5) << std::endl;
    double tmp_var = std::pow(1.0/(_step - _ais->getNumSamplesTrain()) * (_var_sum/(_step - _ais->getNumSamplesTrain()) - std::pow(_pf[0], 2)), 0.5);
    std::cout << "Termm is " << tmp_var << std::endl;
    _cov_pf[0] = tmp_var / _pf[0];
  }
}
