//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppReporterTransfer.h"

registerMooseObject("MooseApp", MultiAppIntegerReporterTransfer);
registerMooseObject("MooseApp", MultiAppRealReporterTransfer);
registerMooseObject("MooseApp", MultiAppVectorReporterTransfer);
registerMooseObject("MooseApp", MultiAppStringReporterTransfer);

template <typename ReporterType>
InputParameters
MultiAppReporterTransfer<ReporterType>::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Transfers reporter data between the master application and sub-application(s).");
  params.addRequiredParam<std::vector<ReporterName>>(
      "from_reporters",
      "List of the reporter names (object_name/value_name) to transfer the value from.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "to_reporters",
      "List of the reporter names (object_name/value_name) to transfer the value to.");
  params.addParam<unsigned int>(
      "subapp_index",
      std::numeric_limits<unsigned int>::max(),
      "The MultiApp object sub-application index to use when transferring to/from the "
      "sub-application. If unset and transferring to the sub-applications then all "
      "sub-applications will receive data. The value must be set when transferring from a "
      "sub-application.");
  return params;
}

template <typename ReporterType>
MultiAppReporterTransfer<ReporterType>::MultiAppReporterTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_reporter_names(getParam<std::vector<ReporterName>>("from_reporters")),
    _to_reporter_names(getParam<std::vector<ReporterName>>("to_reporters")),
    _subapp_index(getParam<unsigned int>("subapp_index"))
{
  if (_from_reporter_names.size() != _to_reporter_names.size())
    paramError("to_reporters", "from_reporters and to_reporters must be the same size.");
}

template <typename ReporterType>
void
MultiAppReporterTransfer<ReporterType>::executeToMultiapp()
{
  for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
  {
    const ReporterType & value =
        _multi_app->problemBase().template getReporterValueByName<ReporterType>(
            _from_reporter_names[n]);

    if (_subapp_index == std::numeric_limits<unsigned int>::max())
    {
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
        if (_multi_app->hasLocalApp(i))
          _multi_app->appProblemBase(i).template setReporterValueByName<ReporterType>(
              _to_reporter_names[n], value);
    }

    else if (_multi_app->hasLocalApp(_subapp_index))
      _multi_app->appProblemBase(_subapp_index)
          .template setReporterValueByName<ReporterType>(_to_reporter_names[n], value);

    else if (_subapp_index >= _multi_app->numGlobalApps())
      paramError(
          "subapp_index",
          "The supplied sub-application index is greater than the number of sub-applications.");
  }
}

template <typename ReporterType>
void
MultiAppReporterTransfer<ReporterType>::executeFromMultiapp()
{
  if (_multi_app->numGlobalApps() > 1 && _subapp_index == std::numeric_limits<unsigned int>::max())
    paramError("multi_app", "subapp_index must be provided when more than one subapp is present.");
  else if (_subapp_index != std::numeric_limits<unsigned int>::max() &&
           _subapp_index >= _multi_app->numGlobalApps())
    paramError(
        "subapp_index",
        "The supplied sub-application index is greater than the number of sub-applications.");

  for (unsigned int n = 0; n < _from_reporter_names.size(); ++n)
  {
    const ReporterType & value =
        _multi_app->appProblemBase(_subapp_index)
            .template getReporterValueByName<ReporterType>(_from_reporter_names[n]);

    _multi_app->problemBase().template setReporterValueByName<ReporterType>(_to_reporter_names[n],
                                                                            value);
  }
}

template <typename ReporterType>
void
MultiAppReporterTransfer<ReporterType>::execute()
{
  _console << "Beginning " << type() << " " << name() << std::endl;
  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
  _console << "Finished " << type() << " " << name() << std::endl;
}
