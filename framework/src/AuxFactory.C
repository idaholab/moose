#include "AuxFactory.h"

AuxFactory::AuxFactory()
  {
    active_NodalAuxKernels.resize(libMesh::n_threads());
    active_ElementAuxKernels.resize(libMesh::n_threads());
    active_bcs.resize(libMesh::n_threads());
  }
 
