//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Backup.h"

#include "DataIO.h"

Backup::Backup()
  : _restartable_data(libMesh::n_threads()), _restartable_data_map(libMesh::n_threads())
{
}

std::stringstream &
Backup::restartableData(const THREAD_ID tid, const WriteKey)
{
  // Writing to _restartable_data invalidates this map
  _restartable_data_map[tid].clear();
  return _restartable_data[tid];
}

void
dataStore(std::ostream & stream, Backup *& backup, void * context)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    dataStore(stream, backup->restartableData(tid, {}), context);
}

void
dataLoad(std::istream & stream, Backup *& backup, void * context)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    dataLoad(stream, backup->restartableData(tid, {}), context);
}
