//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputParameters.h"
#include "Restartable.h"
#include "SubProblem.h"
#include "FEProblem.h"
#include "MooseObject.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "MeshMetaDataInterface.h"

Restartable::Restartable(const MooseObject * moose_object, const std::string & system_name)
  : Restartable(moose_object->getMooseApp(),
                moose_object->name(),
                system_name,
                moose_object->parameters().isParamValid("_tid")
                    ? moose_object->parameters().get<THREAD_ID>("_tid")
                    : 0)
{
}

Restartable::Restartable(const MooseObject * moose_object,
                         const std::string & system_name,
                         THREAD_ID tid)
  : Restartable(moose_object->getMooseApp(), moose_object->name(), system_name, tid)
{
}

Restartable::Restartable(MooseApp & moose_app,
                         const std::string & name,
                         const std::string & system_name,
                         THREAD_ID tid)
  : _restartable_app(moose_app),
    _restartable_system_name(system_name),
    _restartable_tid(tid),
    _restartable_read_only(false),
    _restartable_name(name)
{
}

RestartableDataValue &
Restartable::registerRestartableDataOnApp(const std::string & name,
                                          std::unique_ptr<RestartableDataValue> data,
                                          THREAD_ID tid)
{
  return _restartable_app.registerRestartableData(
      name, std::move(data), tid, _restartable_read_only);
}

void
Restartable::registerRestartableNameWithFilterOnApp(const std::string & name,
                                                    Moose::RESTARTABLE_FILTER filter)
{
  _restartable_app.registerRestartableNameWithFilter(name, filter);
}

std::string
Restartable::restartableName(const std::string & data_name) const
{
  return _restartable_system_name + "/" + _restartable_name + "/" + data_name;
}
