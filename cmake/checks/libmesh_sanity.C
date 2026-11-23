// Sanity check for the MOOSE CMake build: confirms that the MOOSE::libmesh target
// (discovered from libmesh-config) provides enough to compile, link, and run against a
// conda-built libMesh + PETSc + MPI stack.

#include "libmesh/libmesh.h"

#include <cstdio>

int
main(int argc, char ** argv)
{
  libMesh::LibMeshInit init(argc, argv);
  std::puts("libMesh sanity OK: " + std::to_string(libMesh::global_n_processors()) +
            " processor(s)\n");
  return 0;
}
