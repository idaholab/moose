#include "AuxHolder.h"

AuxHolder::AuxHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  active_NodalAuxKernels.resize(libMesh::n_threads());
  active_ElementAuxKernels.resize(libMesh::n_threads());
  active_bcs.resize(libMesh::n_threads());
}

AuxHolder::~AuxHolder()
{
  {
    std::vector<std::vector<AuxKernel *> >::iterator i;
    for(i=active_NodalAuxKernels.begin(); i!=active_NodalAuxKernels.end(); ++i)
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
    for(i=active_ElementAuxKernels.begin(); i!=active_ElementAuxKernels.end(); ++i)
    {
      std::vector<AuxKernel *>::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<AuxKernel *> > >::iterator i;
    for(i=active_bcs.begin(); i!=active_bcs.end(); ++i)
    {
      std::map<unsigned int, std::vector<AuxKernel *> >::iterator j;
      for(j=i->begin(); j!=i->end(); ++j)
      {
        std::vector<AuxKernel *>::iterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeNodalAuxKernelsBegin(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeNodalAuxKernelsEnd(THREAD_ID tid)
{
  return active_NodalAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeElementAuxKernelsBegin(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeElementAuxKernelsEnd(THREAD_ID tid)
{
  return active_ElementAuxKernels[tid].end();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeAuxBCsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].begin();
}

std::vector<AuxKernel *>::iterator
AuxHolder::activeAuxBCsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return active_bcs[tid][boundary_id].end();
}
