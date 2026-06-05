// Phase 0 sanity check for the MOOSE CMake build: confirms that the MOOSE::libmesh target
// (discovered from libmesh-config) provides enough to compile, link, and run against a
// conda-built libMesh + PETSc + MPI stack. Not part of the eventual product.

#include "libmesh/libmesh.h"

#include <iostream>

int
main(int argc, char ** argv)
{
  libMesh::LibMeshInit init(argc, argv);
  std::cout << "libMesh sanity OK: " << libMesh::global_n_processors()
            << " processor(s)" << std::endl;
  return 0;
}
