//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppReporterTransferBase.h"
#include "MultiApp.h"
#include "FEProblemBase.h"

InputParameters
MultiAppReporterTransferBase::validParams()
{
  return MultiAppTransfer::validParams();
}

MultiAppReporterTransferBase::MultiAppReporterTransferBase(const InputParameters & parameters)
  : MultiAppTransfer(parameters)
{
}

void
MultiAppReporterTransferBase::addReporterTransferMode(const ReporterName & name,
                                                      const ReporterMode & mode,
                                                      unsigned int subapp_index)
{
  // For convenience
  FEProblemBase * problem;
  if (subapp_index == std::numeric_limits<unsigned int>::max())
    problem = &_multi_app->problemBase();
  else if (_multi_app->hasLocalApp(subapp_index))
    problem = &_multi_app->appProblemBase(subapp_index);
  else
    return;

  problem->getReporterData(ReporterData::WriteKey()).addConsumerMode(mode, name);
}

void
MultiAppReporterTransferBase::transferToMultiApp(const ReporterName & from_reporter,
                                                 const ReporterName & to_reporter,
                                                 unsigned int subapp_index,
                                                 unsigned int time_index)
{
  if (!_multi_app->hasLocalApp(subapp_index))
    return;
  const ReporterData & from_data = _multi_app->problemBase().getReporterData();
  ReporterData & to_data =
      _multi_app->appProblemBase(subapp_index).getReporterData(ReporterData::WriteKey());
  from_data.transfer(from_reporter, to_reporter, to_data, time_index);
}

void
MultiAppReporterTransferBase::transferFromMultiApp(const ReporterName & from_reporter,
                                                   const ReporterName & to_reporter,
                                                   unsigned int subapp_index,
                                                   unsigned int time_index)
{
  if (!_multi_app->hasLocalApp(subapp_index))
    return;

  const ReporterData & from_data = _multi_app->appProblemBase(subapp_index).getReporterData();
  ReporterData & to_data = _multi_app->problemBase().getReporterData(ReporterData::WriteKey());
  from_data.transfer(from_reporter, to_reporter, to_data, time_index);
}

void
MultiAppReporterTransferBase::execute()
{
  _console << "Beginning " << type() << " " << name() << std::endl;
  if (_current_direction == FROM_MULTIAPP)
    executeFromMultiapp();
  else
    executeToMultiapp();
  _console << "Finished " << type() << " " << name() << std::endl;
}
