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

Backup::Backup(const std::string & filename /* = "" */)
  : _data(libMesh::n_threads()), _data_info_map(libMesh::n_threads()), _filename(filename)
{
}

std::stringstream &
Backup::data(const THREAD_ID tid, const WriteKey)
{
  // Writing to _data invalidates this map
  _data_info_map[tid].clear();
  return _data[tid];
}

void
dataStore(std::ostream & stream, Backup *& backup, void * context)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    dataStore(stream, backup->data(tid, {}), context);
}

void
dataLoad(std::istream & stream, Backup *& backup, void * context)
{
  for (const auto tid : make_range(libMesh::n_threads()))
    dataLoad(stream, backup->data(tid, {}), context);
}
