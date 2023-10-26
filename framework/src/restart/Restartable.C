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
                         THREAD_ID tid,
                         const bool read_only,
                         const RestartableDataMapName & metaname)
  : _restartable_app(moose_app),
    _restartable_system_name(system_name),
    _restartable_tid(tid),
    _restartable_read_only(read_only),
    _metaname(metaname),
    _restartable_name(name)
{
}

RestartableDataValue &
Restartable::registerRestartableDataOnApp(std::unique_ptr<RestartableDataValue> data,
                                          THREAD_ID tid) const
{
  return _restartable_app.registerRestartableData(
      std::move(data), tid, _restartable_read_only, _metaname);
}

void
Restartable::registerRestartableNameWithFilterOnApp(const std::string & name,
                                                    Moose::RESTARTABLE_FILTER filter)
{
  _restartable_app.registerRestartableNameWithFilter(name, filter);
}

std::string
Restartable::restartableName(const std::string & system_name,
                             const std::string & restartable_name,
                             const std::string & data_name)
{
  mooseAssert(data_name.size(), "Should have at least a data name");

  std::string name;
  if (system_name.size())
    name += system_name + "/";
  if (restartable_name.size())
    name += restartable_name + "/";
  name += data_name;
  return name;
}

std::string
Restartable::restartableName(const std::string & object_name, const std::string & data_name) const
{
  return restartableName(_restartable_system_name, object_name, data_name);
}

std::string
Restartable::restartableName(const std::string & data_name) const
{
  return restartableName(_restartable_name, data_name);
}

Restartable::HasRestartableData
Restartable::hasRestartableData(const std::string & full_name, const std::type_info & type) const
{
  // TODO: Add a check that we only call this when we actually have data headers to read.
  // It's not a good idea to allow it to be called in constructors because it'll likely
  // always return false in them because the headers aren't even read yet
  if (const auto map = _restartable_app.queryRestartableDataMap(_metaname, _restartable_tid))
  {
    // Data does exist!
    if (const auto data = map->findData(full_name))
    {
      // And it even has the same type!
      if (data->typeId() == type)
        return HasRestartableData::HAS_DATA_LOADED;

      mooseError("While querying for restartable data '",
                 full_name,
                 "' of type '",
                 MooseUtils::prettyCppType(type),
                 "', said data was found with differing type '",
                 MooseUtils::prettyCppType(data->typeId()),
                 "'");
    }

    // Data doesn't exist, but we also haven't restored anything yet. Thus, we'll probably
    // want to lazily declare it so note that it's not loaded yet
    if (!_restartable_app.hasRestoredRestartableData(_metaname))
      return HasRestartableData::NOT_RESTORED;
    // Data doesn't exist, but it might be restorable late
    if (_restartable_app.getLateRestartableDataRestorer(_metaname).isRestorable(
            full_name, type, _restartable_tid))
      return HasRestartableData::HAS_DATA_RESTORABLE;
  }

  return HasRestartableData::MISSING_DATA;
}

void
Restartable::restoreLateValue(const std::string & full_name)
{
  auto & restorer = _restartable_app.getLateRestartableDataRestorer(_metaname);
  restorer.restore(full_name, _restartable_tid);
}
