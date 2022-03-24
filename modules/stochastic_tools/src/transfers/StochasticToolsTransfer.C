//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "StochasticToolsTransfer.h"
#include "MultiApp.h"
#include "Sampler.h"
#include "SamplerTransientMultiApp.h"
#include "SamplerFullSolveMultiApp.h"

InputParameters
StochasticToolsTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.set<bool>("check_multiapp_execute_on", true) = false; // see comments in constructor
  params.addParam<SamplerName>("sampler", "A the Sampler object that Transfer is associated..");
  return params;
}

StochasticToolsTransfer::StochasticToolsTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters), SamplerInterface(this)
{
  // Since sampler lives in the main app, it's unclear what sibling transfer should look like
  if (hasFromMultiApp() && hasToMultiApp())
    mooseError("Transfers between multiapp are not currently supported for this transfer type");

  const auto multi_app = hasFromMultiApp() ? getFromMultiApp() : getToMultiApp();

  // When the MultiApp is running in batch mode the execute flags for the transfer object must
  // be removed. If not the 'regular' transfer that occurs will potentially destroy data
  // populated during the calls from the MultiApp in batch mode. To prevent the Transfer from
  // running the execute flags must be removed. This is done automatically here, unless
  // 'execute_on' was modified by the user, which an error is produced.
  if (multi_app->isParamValid("mode") &&
      (multi_app->getParam<MooseEnum>("mode") == "batch-reset" ||
       multi_app->getParam<MooseEnum>("mode") == "batch-restore"))
  {
    if (parameters.isParamSetByUser("execute_on"))
      paramError("execute_on",
                 "The 'execute_on' parameter for the '",
                 name(),
                 "' transfer was set, but the parent MultiApp object (",
                 multi_app->name(),
                 ") is running in 'batch' mode. For this case the 'execute_on' parameter must not "
                 "be set by the user or set to NONE.");
    else
    {
      ExecFlagEnum & exec_flags = const_cast<ExecFlagEnum &>(getParam<ExecFlagEnum>("execute_on"));
      exec_flags = EXEC_NONE;
    }
  }

  // In the validParams method above the 'check_multiapp_execute_on' is disabled. This is required
  // to allow for the error above to be triggered. If the 'execute_on' is set by the without
  // the 'check_multiapp_execute_on' flag the above if statement may not be reached. Therefore,
  // a bit of a trick is performed to allow the above check to run first and then the regular
  // check.
  //
  // If the else statement is reached then the user is not running in batch mode, so the
  // 'check_multiapp_execute_on' is a valid check to perform. If the 'check_multiapp_execute_on'
  // has not been set, then the user wants the check to be performed, so do it.
  else if (!parameters.isParamSetByUser("check_multiapp_execute_on"))
    checkMultiAppExecuteOn();

  // Determine the Sampler
  if (isParamValid("sampler"))
  {
    _sampler_ptr = &(getSampler("sampler"));

    SamplerTransientMultiApp * ptr_transient =
        dynamic_cast<SamplerTransientMultiApp *>(multi_app.get());
    SamplerFullSolveMultiApp * ptr_fullsolve =
        dynamic_cast<SamplerFullSolveMultiApp *>(multi_app.get());

    if (!ptr_transient && !ptr_fullsolve)
      mooseError("The 'multi_app' parameter must provide either a 'SamplerTransientMultiApp' or "
                 "'SamplerFullSolveMultiApp' object.");

    if ((ptr_transient && &(ptr_transient->getSampler("sampler")) != _sampler_ptr) ||
        (ptr_fullsolve && &(ptr_fullsolve->getSampler("sampler")) != _sampler_ptr))
      mooseError("The supplied 'multi_app' must have the same Sampler object as this Transfer.");
  }

  else
  {
    paramWarning("sampler",
                 "Support for the 'StochasticToolsTransfer' objects without the 'sampler' input "
                 "parameter is being removed, please update your input file(s).");

    std::shared_ptr<SamplerTransientMultiApp> ptr_transient =
        std::dynamic_pointer_cast<SamplerTransientMultiApp>(multi_app);
    std::shared_ptr<SamplerFullSolveMultiApp> ptr_fullsolve =
        std::dynamic_pointer_cast<SamplerFullSolveMultiApp>(multi_app);

    if (!ptr_transient && !ptr_fullsolve)
      mooseError("The 'multi_app' parameter must provide either a 'SamplerTransientMultiApp' or "
                 "'SamplerFullSolveMultiApp' object.");

    if (ptr_transient)
      _sampler_ptr = &(ptr_transient->getSampler("sampler"));
    else
      _sampler_ptr = &(ptr_fullsolve->getSampler("sampler"));
  }
}

void
StochasticToolsTransfer::initializeFromMultiapp()
{
}

void
StochasticToolsTransfer::executeFromMultiapp()
{
}

void
StochasticToolsTransfer::finalizeFromMultiapp()
{
}

void
StochasticToolsTransfer::initializeToMultiapp()
{
}

void
StochasticToolsTransfer::executeToMultiapp()
{
}

void
StochasticToolsTransfer::finalizeToMultiapp()
{
}
