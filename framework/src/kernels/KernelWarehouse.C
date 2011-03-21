#include "KernelWarehouse.h"

KernelWarehouse::KernelWarehouse()
{
}

KernelWarehouse::~KernelWarehouse()
{
  for (KernelIterator i=_all_kernels.begin(); i!=_all_kernels.end(); ++i)
    delete *i;
}

void
KernelWarehouse::addKernel(Kernel *kernel, const std::set<unsigned int> & block_ids)
{
  _all_kernels.push_back(kernel);
  for (std::set<unsigned int>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
  {
    unsigned int blk_id = *it;
    if (blk_id == Moose::ANY_BLOCK_ID)
      _global_kernels.push_back(kernel);
    else
      _block_kernels[blk_id].push_back(kernel);
  }
}

KernelIterator
KernelWarehouse::allKernelsBegin()
{
  return _all_kernels.begin();
}

KernelIterator
KernelWarehouse::allKernelsEnd()
{
  return _all_kernels.end();
}

KernelIterator
KernelWarehouse::activeKernelsBegin()
{
  return _active_kernels.begin();
}

KernelIterator
KernelWarehouse::activeKernelsEnd()
{
  return _active_kernels.end();
}

void
KernelWarehouse::updateActiveKernels(Real t, Real dt, unsigned int subdomain_id)
{
  _active_kernels.clear();

  // add kernels that live everywhere
  for (KernelIterator it = _global_kernels.begin(); it != _global_kernels.end(); ++it)
    if((*it)->startTime() <= t + (1e-6 * dt) && (*it)->stopTime() >= t + (1e-6 * dt))
      _active_kernels.push_back(*it);
  // then kernels that live on a specified block
  for (KernelIterator it = _block_kernels[subdomain_id].begin(); it != _block_kernels[subdomain_id].end(); ++it)
    if((*it)->startTime() <= t + (1e-6 * dt) && (*it)->stopTime() >= t + (1e-6 * dt))
      _active_kernels.push_back(*it);
}

bool
KernelWarehouse::subdomains_covered(std::set<unsigned int> & return_set) const
{
  return_set.clear();
  
  if (!_global_kernels.empty())
    return true;
  else
  {
    for (std::map<unsigned int, std::vector<Kernel *> >::const_iterator it = _block_kernels.begin();
         it != _block_kernels.end(); ++it)
      return_set.insert(it->first);
    return false;
  }
}
