#include "SerializerGuard.h"

#include "MooseUtils.h"

#include "libmesh/parallel.h"

SerializerGuard::SerializerGuard(const libMesh::Parallel::Communicator & comm, bool warn)
  : _comm(comm), _warn(warn)
{
  MooseUtils::serialBegin(_comm, _warn);
}

SerializerGuard::~SerializerGuard() { MooseUtils::serialEnd(_comm, _warn); }
