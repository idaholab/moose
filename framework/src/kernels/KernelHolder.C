#include "KernelHolder.h"
#include "MooseSystem.h"

KernelHolder::KernelHolder(MooseSystem &sys) :
  _moose_system(sys)
{
  active_kernels.resize(libMesh::n_threads());
  all_kernels.resize(libMesh::n_threads());
  block_kernels.resize(libMesh::n_threads());
  all_block_kernels.resize(libMesh::n_threads());
}

KernelHolder::~KernelHolder()
{
  {

    std::vector<std::vector<Kernel *> >::iterator i;
    for (i=active_kernels.begin(); i!=active_kernels.end(); ++i)
    {

      KernelIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  {
    std::vector<std::map<unsigned int, std::vector<Kernel *> > >::iterator i;
    for (i=block_kernels.begin(); i!=block_kernels.end(); ++i)
    {

      std::map<unsigned int, std::vector<Kernel *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        KernelIterator k;
        for(k=(j->second).begin(); k!=(j->second).end(); ++k)
        {
          delete *k;
        }
      }
    }
  }
}

KernelIterator
KernelHolder::activeKernelsBegin(THREAD_ID tid)
{
  return active_kernels[tid].begin();
}

KernelIterator
KernelHolder::activeKernelsEnd(THREAD_ID tid)
{
  return active_kernels[tid].end();
}


KernelIterator
KernelHolder::blockKernelsBegin(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].begin();
}

KernelIterator
KernelHolder::blockKernelsEnd(THREAD_ID tid, unsigned int block_id)
{
  return block_kernels[tid][block_id].end();
}

bool
KernelHolder::activeKernelBlocks(std::set<subdomain_id_type> & set_buffer) const
{
  std::map<unsigned int, std::vector<Kernel *> >::const_iterator curr, end;
  end = block_kernels[0].end();

  try
  {
    for (curr = block_kernels[0].begin(); curr != end; ++curr)
      set_buffer.insert(subdomain_id_type(curr->first));
  }
  catch (std::exception &e)
  {
    mooseError("Invalid block specified in input file");
  }

  // return a boolean indicated whether there are any global kernels active
  return ! active_kernels[0].empty();
}

void
KernelHolder::updateActiveKernels(THREAD_ID tid)
{
  {
    Real t = _moose_system._t;

    if(t >= 1.0)
    {
      t++;
    }


    active_kernels[tid].clear();

    KernelIterator all_it = all_kernels[tid].begin();
    KernelIterator all_end = all_kernels[tid].end();

    for(; all_it != all_end; ++all_it)
      if((*all_it)->startTime() <= _moose_system._t + (1e-6 * _moose_system._dt) && (*all_it)->stopTime() >= _moose_system._t + (1e-6 * _moose_system._dt))
        active_kernels[tid].push_back(*all_it);
  }

  {
    block_kernels[tid].clear();

    std::map<unsigned int, std::vector<Kernel *> >::iterator block_it = all_block_kernels[tid].begin();
    std::map<unsigned int, std::vector<Kernel *> >::iterator block_end = all_block_kernels[tid].end();

    for(; block_it != block_end; ++block_it)
    {
      unsigned int block_num = block_it->first;
      block_kernels[tid][block_num].clear();

      KernelIterator all_block_it = block_it->second.begin();
      KernelIterator all_block_end = block_it->second.end();

      for(; all_block_it != all_block_end; ++all_block_it)
        if((*all_block_it)->startTime() <= _moose_system._t + (1e-6 * _moose_system._dt) && (*all_block_it)->stopTime() >= _moose_system._t + (1e-6 * _moose_system._dt))
          block_kernels[tid][block_num].push_back(*all_block_it);
    }
  }
}
