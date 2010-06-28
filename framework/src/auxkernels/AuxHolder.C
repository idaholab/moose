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
      AuxKernelIterator j;
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
      AuxKernelIterator j;
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
      AuxKernelIterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }
}

AuxKernelIterator
AuxHolder::activeNodalAuxKernelsBegin(THREAD_ID tid)
{
  return _active_nodal_aux_kernels[tid].begin();
}

AuxKernelIterator
AuxHolder::activeNodalAuxKernelsEnd(THREAD_ID tid)
{
  return _active_nodal_aux_kernels[tid].end();
}

AuxKernelIterator
AuxHolder::activeElementAuxKernelsBegin(THREAD_ID tid)
{
  return _active_element_aux_kernels[tid].begin();
}

AuxKernelIterator
AuxHolder::activeElementAuxKernelsEnd(THREAD_ID tid)
{
  return _active_element_aux_kernels[tid].end();
}

AuxKernelIterator
AuxHolder::activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].begin();
}

AuxKernelIterator
AuxHolder::activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _active_bcs[tid][boundary_id].end();
}

std::list<AuxKernel *>
AuxHolder::getActiveNodalKernels(THREAD_ID tid)
{
  return std::list<AuxKernel *>(_active_nodal_aux_kernels[tid].begin(), _active_nodal_aux_kernels[tid].end());
}

std::list<AuxKernel *>
AuxHolder::getActiveElementKernels(THREAD_ID tid)
{
  return std::list<AuxKernel *>(_active_element_aux_kernels[tid].begin(), _active_element_aux_kernels[tid].end());
}

void
AuxHolder::setActiveNodalKernels(THREAD_ID tid, std::list<AuxKernel *> &auxs)
{
  _active_nodal_aux_kernels[tid].assign(auxs.begin(), auxs.end());
}

void
AuxHolder::setActiveElementKernels(THREAD_ID tid, std::list<AuxKernel *> &auxs)
{
  _active_element_aux_kernels[tid].assign(auxs.begin(), auxs.end());
}

void
AuxHolder::addBC(THREAD_ID tid, AuxKernel *aux)
{
  _aux_bcs[tid].push_back(aux);
}

void
AuxHolder::addActiveBC(THREAD_ID tid, unsigned int boundary_id, AuxKernel *aux)
{
  _active_bcs[tid][boundary_id].push_back(aux);
}
