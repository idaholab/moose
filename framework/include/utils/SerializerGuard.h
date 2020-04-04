//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace libMesh
{
namespace Parallel
{
class Communicator;
}
}

/**
 * A scope guard that guarantees that whatever happens between when it gets created and when it is
 * destroyed is done "serially" (each MPI rank will run in turn starting from 0)
 */
class SerializerGuard
{
public:
  SerializerGuard(const libMesh::Parallel::Communicator & comm, bool warn = true);
  ~SerializerGuard();

protected:
  const libMesh::Parallel::Communicator & _comm;
  bool _warn;
};
