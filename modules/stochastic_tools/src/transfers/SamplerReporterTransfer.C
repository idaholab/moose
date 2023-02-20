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
#include "StochasticReporter.h"
#include "Executioner.h"

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

  params.suppressParameter<MultiMooseEnum>("direction");
  params.suppressParameter<MultiAppName>("multi_app");
  return params;
}

SamplerReporterTransfer::SamplerReporterTransfer(const InputParameters & parameters)
  : StochasticToolsTransfer(parameters),
    ReporterTransferInterface(this),
    _sub_reporter_names(getParam<std::vector<ReporterName>>("from_reporter"))
{
  if (hasToMultiApp())
    paramError("to_multi_app", "To and between multiapp directions are not implemented");
}

void
SamplerReporterTransfer::initialSetup()
{
  // Get the StochasticResults VPP object to populate
  auto & uo = _fe_problem.getUserObject<UserObject>(getParam<std::string>("stochastic_reporter"));
  _results = dynamic_cast<StochasticReporter *>(&uo);
  if (!_results)
    paramError("stochastic_reporter", "This object must be a 'StochasticReporter' object.");

  intitializeStochasticReporters();
}

void
SamplerReporterTransfer::initializeFromMultiapp()
{
}

void
SamplerReporterTransfer::executeFromMultiapp()
{
  if (getFromMultiApp()->isRootProcessor())
  {
    const dof_id_type n = getFromMultiApp()->numGlobalApps();
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
  for (dof_id_type i = _sampler_ptr->getLocalRowBegin(); i < _sampler_ptr->getLocalRowEnd(); ++i)
    transferStochasticReporters(i, i);
}

void
SamplerReporterTransfer::intitializeStochasticReporters()
{
  const dof_id_type n = getFromMultiApp()->numGlobalApps();

  for (const auto & sub_rname : _sub_reporter_names)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (getFromMultiApp()->hasLocalApp(i))
        addReporterTransferMode(
            sub_rname, REPORTER_MODE_ROOT, getFromMultiApp()->appProblemBase(i));

  const std::string prefix = isParamValid("prefix") ? getParam<std::string>("prefix") : name();
  for (const auto & sub_rname : _sub_reporter_names)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (getFromMultiApp()->hasLocalApp(i))
      {
        const ReporterData & rdata = getFromMultiApp()->appProblemBase(i).getReporterData();
        ReporterName rname =
            _results->declareStochasticReporterClone(*_sampler_ptr, rdata, sub_rname, prefix);
        if (rname.empty())
          paramError("from_reporter",
                     "Reporter value ",
                     sub_rname,
                     " is of unsupported type ",
                     rdata.getReporterContextBase(sub_rname).type(),
                     ". Contact MOOSE developers on how to transfer this type of reporter value.");
        _reporter_names.push_back(rname);
        break;
      }

  _converged = &_results->declareStochasticReporter<bool>(
      prefix + (prefix.empty() ? "" : ":") + "converged", *_sampler_ptr);
}

void
SamplerReporterTransfer::transferStochasticReporters(dof_id_type global_index,
                                                     dof_id_type app_index)
{
  if (getFromMultiApp()->hasLocalApp(app_index))
  {
    const dof_id_type local_index = global_index - _sampler_ptr->getLocalRowBegin();
    for (unsigned int r = 0; r < _sub_reporter_names.size(); ++r)
      transferToVectorReporter(_sub_reporter_names[r],
                               _reporter_names[r],
                               getFromMultiApp()->appProblemBase(app_index),
                               getFromMultiApp()->problemBase(),
                               local_index);

    (*_converged)[local_index] = getFromMultiApp()->getExecutioner(app_index)->lastSolveConverged();
  }
}
