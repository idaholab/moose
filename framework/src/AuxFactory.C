#include "AuxFactory.h"
 

 Parameters
 AuxFactory::getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Aux "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

std::vector<AuxKernel *>::iterator
AuxFactory::activeNodalAuxKernelsBegin(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeNodalAuxKernelsEnd(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeElementAuxKernelsBegin(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeElementAuxKernelsEnd(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].begin();
}

std::vector<AuxKernel *>::iterator
AuxFactory::activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].end();
}
  
AuxFactory::AuxFactory()
  {
    active_NodalAuxKernels.resize(libMesh::n_threads());
    active_ElementAuxKernels.resize(libMesh::n_threads());
    active_bcs.resize(libMesh::n_threads());
  }

