//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppCloneReporterTransfer.h"
#include "MultiApp.h"
#include "UserObject.h"
#include "Reporter.h"

registerMooseObject("MooseApp", MultiAppCloneReporterTransfer);

InputParameters
MultiAppCloneReporterTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params += ReporterTransferInterface::validParams();
  params.addClassDescription(
      "Declare and transfer reporter data from sub-application(s) to main application.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "from_reporters",
      "List of the reporter names (object_name/value_name) to transfer the value from.");
  params.addRequiredParam<std::string>(
      "to_reporter", "Reporter object to reference when declaring reporter values.");

  params.addParam<std::string>("prefix",
                               "Use the supplied string as the prefix for reporter "
                               "name rather than the transfer name.");

  params.addParam<MultiMooseEnum>(
      "reporter_type",
      standardTransferTypes(),
      "The type of the reporter on the sub-application. This parameter is not typically required, "
      "but if some processors do not have a sub-app, for instance if the max_procs_per_app "
      "parameter is used in the MultiApp, then this is required.");

  params.suppressParameter<MultiMooseEnum>("direction");
  return params;
}

MultiAppCloneReporterTransfer::MultiAppCloneReporterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    ReporterTransferInterface(this),
    _from_reporter_names(getParam<std::vector<ReporterName>>("from_reporters")),
    _to_obj_name(getParam<std::string>("to_reporter")),
    _to_reporter_names(isParamValid("prefix")
                           ? getReporterNamesHelper(getParam<std::string>("prefix"),
                                                    _to_obj_name,
                                                    _from_reporter_names)
                           : getReporterNamesHelper(_name, _to_obj_name, _from_reporter_names))
{
  // Deprecating direction proves fatal for this parameter for this class
  if (isParamValid("multi_app"))
    paramError("multi_app",
               "The multi_app parameter is no longer valid for this class, use to_multi_app");

  if (isParamValid("to_multi_app"))
    paramError("to_multi_app",
               "Sibling or to_multiapp transfer have not been implemented for this transfer.");
}

void
MultiAppCloneReporterTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  if (isParamValid("to_multi_app") && !getToMultiApp()->hasApp() && !isParamValid("reporter_type"))
    mooseError("For a direct reporter clone, all processors must be associated with a "
               "sub-application. If you know the type of reporter being transferred, please "
               "consider using the 'reporter_type' parameter for an indirect clone.");

  if (isParamValid("from_multi_app") && !getFromMultiApp()->hasApp() &&
      !isParamValid("reporter_type"))
    mooseError("For a direct reporter clone, all processors must be associated with a "
               "sub-application. If you know the type of reporter being transferred, please "
               "consider using the 'reporter_type' parameter for an indirect clone.");

  const UserObject & uo = _fe_problem.getUserObjectBase(_to_obj_name);
  if (!dynamic_cast<const Reporter *>(&uo))
    paramError("to_reporter", "This object must be a Reporter object.");

  const auto multi_app = getMultiApp();
  const dof_id_type n = multi_app->numGlobalApps();

  for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (multi_app->hasLocalApp(i))
        addReporterTransferMode(
            _from_reporter_names[r], REPORTER_MODE_ROOT, multi_app->appProblemBase(i));

  if (multi_app->hasApp())
  {
    for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
      for (MooseIndex(n) i = 0; i < n; i++)
        if (multi_app->hasLocalApp(i))
        {
          if (n > 1)
            declareVectorClone(_from_reporter_names[r],
                               _to_reporter_names[r],
                               multi_app->appProblemBase(i),
                               multi_app->problemBase(),
                               REPORTER_MODE_DISTRIBUTED);
          else
            declareClone(_from_reporter_names[r],
                         _to_reporter_names[r],
                         multi_app->appProblemBase(i),
                         multi_app->problemBase(),
                         REPORTER_MODE_ROOT);
          break;
        }
  }
  else
  {
    const auto & types = getParam<MultiMooseEnum>("reporter_type");
    if (types.size() != _from_reporter_names.size())
      paramError("reporter_type", "This parameter must be the same length as 'from_reporters'");
    for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    {
      if (n > 1)
        declareVectorClone(
            _to_reporter_names[r], multi_app->problemBase(), types[r], REPORTER_MODE_DISTRIBUTED);
      else
        declareClone(_to_reporter_names[r], multi_app->problemBase(), types[r], REPORTER_MODE_ROOT);
    }
  }

  if (n > 1 && multi_app->isRootProcessor())
    for (const auto & rn : _to_reporter_names)
      resizeReporter(rn, multi_app->problemBase(), multi_app->numLocalApps());
}

void
MultiAppCloneReporterTransfer::executeToMultiapp()
{
}

void
MultiAppCloneReporterTransfer::executeFromMultiapp()
{
  if (!getFromMultiApp()->isRootProcessor())
    return;

  const dof_id_type begin = getFromMultiApp()->firstLocalApp();
  const dof_id_type end = begin + getFromMultiApp()->numLocalApps();

  for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    for (dof_id_type i = begin; i < end; ++i)
    {
      if (getFromMultiApp()->numGlobalApps() > 1)
        transferToVectorReporter(_from_reporter_names[r],
                                 _to_reporter_names[r],
                                 getFromMultiApp()->appProblemBase(i),
                                 getFromMultiApp()->problemBase(),
                                 i - begin);
      else
        transferReporter(_from_reporter_names[r],
                         _to_reporter_names[r],
                         getFromMultiApp()->appProblemBase(i),
                         getFromMultiApp()->problemBase());
    }
}

void
MultiAppCloneReporterTransfer::execute()
{
  TIME_SECTION("MultiAppCloneReporterTransfer::execute()", 5, "Transferring reporters");

  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
}
