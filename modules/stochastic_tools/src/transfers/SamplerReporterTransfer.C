//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// StochasticTools includes
#include "SamplerReporterTransfer.h"
#include "SamplerFullSolveMultiApp.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerReceiver.h"
#include "StochasticResults.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SamplerReporterTransfer);

InputParameters
SamplerReporterTransfer::validParams()
{
  InputParameters params = StochasticToolsTransfer::validParams();
  params.addClassDescription("Transfers data from Reporters on the sub-application to a "
                             "StochasticReporter on the main application.");

  params.addRequiredParam<std::vector<ReporterName>>(
      "from_reporter", "The name(s) of the Reporter(s) on the sub-app to transfer from.");
  params.addRequiredParam<std::string>(
      "stochastic_reporter", "The name of the StochasticReporter object to transfer values to.");

  params.addParam<std::string>("prefix",
                               "Use the supplied string as the prefix for reporter "
                               "name rather than the transfer name.");

  params.set<MultiMooseEnum>("direction") = "from_multiapp";
  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

SamplerReporterTransfer::SamplerReporterTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    ReporterTransferInterface(this),
    _sub_reporter_names(getParam<std::vector<ReporterName>>("from_reporter")),
    _sr_name(getParam<std::string>("stochastic_reporter")),
    _reporter_names(
        isParamValid("prefix")
            ? getReporterNamesHelper(getParam<std::string>("prefix"), _sr_name, _sub_reporter_names)
            : getReporterNamesHelper(_name, _sr_name, _sub_reporter_names)),
    _converged_name(_sr_name, StochasticReporter::convergedReporterName())
{
}

void
SamplerReporterTransfer::initialSetup()
{
  // Get the StochasticResults VPP object to populate
  auto & uo = _fe_problem.getUserObject<UserObject>(_sr_name);
  _results = dynamic_cast<StochasticReporter *>(&uo);
  if (!_results)
    paramError("stochastic_reporter", "This object must be a 'StochasticReporter' object.");

  intitializeStochasticReporters();
}

void
SamplerReporterTransfer::initializeFromMultiapp()
{
  resizeStochasticReporters();
}

void
SamplerReporterTransfer::executeFromMultiapp()
{
  if (_multi_app->isRootProcessor())
  {
    const dof_id_type n = _multi_app->numGlobalApps();
    for (MooseIndex(n) i = 0; i < n; i++)
      transferStochasticReporters(_global_index, i);
  }
}

void
SamplerReporterTransfer::finalizeFromMultiapp()
{
}

void
SamplerReporterTransfer::execute()
{
  resizeStochasticReporters();

  for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
    transferStochasticReporters(i, i);
}

void
SamplerReporterTransfer::intitializeStochasticReporters()
{
  const dof_id_type n = _multi_app->numGlobalApps();

  for (unsigned int r = 0; r < _sub_reporter_names.size(); ++r)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (_multi_app->hasLocalApp(i))
        addReporterTransferMode(
            _sub_reporter_names[r], REPORTER_MODE_ROOT, _multi_app->appProblemBase(i));

  for (unsigned int r = 0; r < _sub_reporter_names.size(); ++r)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (_multi_app->hasLocalApp(i))
      {
        declareVectorClone(_sub_reporter_names[r],
                           _reporter_names[r],
                           _multi_app->appProblemBase(i),
                           _multi_app->problemBase(),
                           REPORTER_MODE_DISTRIBUTED);
        break;
      }
}

void
SamplerReporterTransfer::resizeStochasticReporters()
{
  for (const auto & rn : _reporter_names)
    resizeReporter(rn, _multi_app->problemBase(), _sampler_ptr->getNumberOfLocalRows());

  resizeReporter(_converged_name, _multi_app->problemBase(), _sampler_ptr->getNumberOfLocalRows());
}

void
SamplerReporterTransfer::transferStochasticReporters(dof_id_type global_index,
                                                     dof_id_type app_index)
{
  if (_multi_app->hasLocalApp(app_index))
  {
    const dof_id_type local_index = global_index - _sampler_ptr->getLocalRowBegin();
    for (unsigned int r = 0; r < _sub_reporter_names.size(); ++r)
      transferToVectorReporter(_sub_reporter_names[r],
                               _reporter_names[r],
                               _multi_app->appProblemBase(app_index),
                               _multi_app->problemBase(),
                               local_index);

    _results->setSampleConverged(_multi_app->appProblemBase(app_index).converged(), local_index);
  }
}
