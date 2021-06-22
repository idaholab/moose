//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUtils.h"

// MPI

const libMesh::Parallel::Communicator * AbaqusUtils::_communicator = nullptr;

void
AbaqusUtils::setCommunicator(const libMesh::Parallel::Communicator * communicator)
{
  if (_communicator != nullptr && communicator != _communicator)
    mooseWarning("Conflicting MPI communicators specified in Abaqus compatibility objects. Are you "
                 "running a multiapps simulation?");

  _communicator = communicator;
}

void
getnumcpus_(int * num)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *num = communicator->size();
}

// Threads

void
getrank_(int * rank)
{
  auto communicator = AbaqusUtils::getCommunicator();
  *rank = communicator->rank();
}

int
getnumthreads_()
{
#if defined(LIBMESH_HAVE_TBB_API) ||                                                               \
    (!defined(LIBMESH_HAVE_OPENMP) && defined(LIBMESH_HAVE_PTHREAD))
  return libMesh::n_threads();
#else
  return 1;
#endif
}

int
get_thread_id_()
{
  ParallelUniqueId puid;
  return puid.id;
}

// Output directory

std::string AbaqusUtils::_output_dir = "";

void
AbaqusUtils::setOutputDir(const std::string & output_dir)
{
  if (!output_dir.empty() && output_dir != _output_dir)
    mooseWarning("Conflicting output directories specified in Abaqus compatibility objects: ",
                 output_dir,
                 " != ",
                 _output_dir,
                 ". Are you running a multiapps simulation?");

  _output_dir = output_dir;
}

void
getoutdir_(char * dir, unsigned int * len)
{
  auto output_dir = AbaqusUtils::getOutputDir();
  *len = output_dir.length();
  for (unsigned int i = 0; i < 256; ++i)
    dir[i] = i < *len ? output_dir[i] : ' ';
}
