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

  params.set<MultiMooseEnum>("direction") = "from_multiapp";
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
}

void
MultiAppCloneReporterTransfer::initialSetup()
{
  const UserObject & uo = _fe_problem.getUserObjectBase(_to_obj_name);
  // auto & uo = _fe_problem.getUserObject<UserObject>(_to_obj_name);
  if (!dynamic_cast<const Reporter *>(&uo))
    paramError("to_reporter", "This object must be a Reporter object.");

  const dof_id_type n = _multi_app->numGlobalApps();

  for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (_multi_app->hasLocalApp(i))
        addReporterTransferMode(
            _from_reporter_names[r], REPORTER_MODE_ROOT, _multi_app->appProblemBase(i));

  for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    for (MooseIndex(n) i = 0; i < n; i++)
      if (_multi_app->hasLocalApp(i))
      {
        if (n > 1)
          declareVectorClone(_from_reporter_names[r],
                             _to_reporter_names[r],
                             _multi_app->appProblemBase(i),
                             _multi_app->problemBase(),
                             REPORTER_MODE_DISTRIBUTED);
        else
          declareClone(_from_reporter_names[r],
                       _to_reporter_names[r],
                       _multi_app->appProblemBase(i),
                       _multi_app->problemBase(),
                       REPORTER_MODE_ROOT);
        break;
      }

  if (n > 1 && _multi_app->isRootProcessor())
    for (const auto & rn : _to_reporter_names)
      resizeReporter(rn, _multi_app->problemBase(), _multi_app->numLocalApps());
}

void
MultiAppCloneReporterTransfer::executeToMultiapp()
{
}

void
MultiAppCloneReporterTransfer::executeFromMultiapp()
{
  if (!_multi_app->isRootProcessor())
    return;

  const dof_id_type begin = _multi_app->firstLocalApp();
  const dof_id_type end = begin + _multi_app->numLocalApps();

  for (unsigned int r = 0; r < _from_reporter_names.size(); ++r)
    for (dof_id_type i = begin; i < end; ++i)
    {
      if (_multi_app->numGlobalApps() > 1)
        transferToVectorReporter(_from_reporter_names[r],
                                 _to_reporter_names[r],
                                 _multi_app->appProblemBase(i),
                                 _multi_app->problemBase(),
                                 i - begin);
      else
        transferReporter(_from_reporter_names[r],
                         _to_reporter_names[r],
                         _multi_app->appProblemBase(i),
                         _multi_app->problemBase());
    }
}

void
MultiAppCloneReporterTransfer::execute()
{
  _console << "Beginning " << type() << " " << name() << std::endl;
  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
  _console << "Finished " << type() << " " << name() << std::endl;
}
