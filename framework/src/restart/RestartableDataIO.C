//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableDataIO.h"

#include "MooseApp.h"

const std::string RestartableDataIO::RESTARTABLE_DATA_EXT = ".rd";
const unsigned int RestartableDataIO::CURRENT_BACKUP_FILE_VERSION = 3;

RestartableDataIO::RestartableDataIO(MooseApp & app, RestartableDataMap & data)
  : PerfGraphInterface(app.perfGraph(), "RestartableDataIO"),
    libMesh::ParallelObject(app),
    _data(&data)
{
}

RestartableDataIO::RestartableDataIO(MooseApp & app, std::vector<RestartableDataMap> & data)
  : PerfGraphInterface(app.perfGraph(), "RestartableDataIO"),
    libMesh::ParallelObject(app),
    _data(&data)
{
}

RestartableDataMap &
RestartableDataIO::currentData(const THREAD_ID tid)
{
  mooseAssert(dataSize() > tid, "Invalid thread");

  if (std::holds_alternative<RestartableDataMap *>(_data))
    return *std::get<RestartableDataMap *>(_data);
  return (*std::get<std::vector<RestartableDataMap> *>(_data))[tid];
}

std::size_t
RestartableDataIO::dataSize() const
{
  return std::holds_alternative<RestartableDataMap *>(_data)
             ? 1
             : std::get<std::vector<RestartableDataMap> *>(_data)->size();
}
