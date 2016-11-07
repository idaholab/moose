/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
