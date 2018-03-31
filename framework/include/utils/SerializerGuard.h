#ifndef SERIALIZERGUARD_H
#define SERIALIZERGUARD_H

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

#endif
