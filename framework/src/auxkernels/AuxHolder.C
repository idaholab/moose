#include "AuxHolder.h"

AuxHolder::AuxHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_nodal_aux_kernels.resize(libMesh::n_threads());
  _active_element_aux_kernels.resize(libMesh::n_threads());
  _active_bcs.resize(libMesh::n_threads());
  _aux_bcs.resize(libMesh::n_threads());
}

AuxHolder::~AuxHolder()
{
  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=_active_nodal_aux_kernels.begin(); i!=_active_nodal_aux_kernels.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=_active_element_aux_kernels.begin(); i!=_active_element_aux_kernels.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=_aux_bcs.begin(); i!=_aux_bcs.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeNodalAuxKernelsBegin(THREAD_ID tid)
{
  return _active_nodal_aux_kernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeNodalAuxKernelsEnd(THREAD_ID tid)
{
  return _active_nodal_aux_kernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeElementAuxKernelsBegin(THREAD_ID tid)
{
  return _active_element_aux_kernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeElementAuxKernelsEnd(THREAD_ID tid)
{
  return _active_element_aux_kernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].end();
}
