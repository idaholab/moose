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

const std::string &
RestartableDataIO::getRestartableExt()
{
  static const std::string ext = ".rd";
  return ext;
}

const std::string &
RestartableDataIO::restartableDataFile()
{
  static const std::string file = "data";
  return file;
}

const std::string &
RestartableDataIO::restartableHeaderFile()
{
  static const std::string file = "header";
  return file;
}

std::filesystem::path
RestartableDataIO::restartableDataFolder(const std::filesystem::path & folder_base)
{
  auto folder = folder_base;
  folder += getRestartableExt();
  return folder;
}

std::filesystem::path
RestartableDataIO::restartableDataFile(const std::filesystem::path & folder_base)
{
  return folder_base / restartableDataFile();
}

std::filesystem::path
RestartableDataIO::restartableHeaderFile(const std::filesystem::path & folder_base)
{
  return folder_base / restartableHeaderFile();
}
