//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Backup.h"
#include "RestartableData.h"

#include "libmesh/parallel.h"

// Backup Definitions
Backup::Backup()
{
  unsigned int n_threads = libMesh::n_threads();

  _restartable_data.resize(n_threads);

  for (unsigned int i = 0; i < n_threads; ++i)
    _restartable_data[i] = new std::stringstream;
}

Backup::~Backup()
{
  unsigned int n_threads = libMesh::n_threads();

  for (unsigned int i = 0; i < n_threads; ++i)
    delete _restartable_data[i];
}
