/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "KernelWarehouse.h"
#include "MooseSystem.h"

KernelWarehouse::KernelWarehouse()
{
}

KernelWarehouse::~KernelWarehouse()
{
  for (KernelIterator i=_active_kernels.begin(); i!=_active_kernels.end(); ++i)
    delete *i;

  for (std::map<unsigned int, std::vector<Kernel *> >::iterator i=_block_kernels.begin(); i!=_block_kernels.end(); ++i)
    for(KernelIterator j=(i->second).begin(); j!=(i->second).end(); ++j)
      delete *j;
}

void
KernelWarehouse::addKernel(Kernel *kernel)
{
  _all_kernels.push_back(kernel);
}

void
KernelWarehouse::addBlockKernel(unsigned int block_id, Kernel *kernel)
{
  _all_block_kernels[block_id].push_back(kernel);
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


KernelIterator
KernelWarehouse::blockKernelsBegin(unsigned int block_id)
{
  return _block_kernels[block_id].begin();
}

KernelIterator
KernelWarehouse::blockKernelsEnd(unsigned int block_id)
{
  return _block_kernels[block_id].end();
}

bool
KernelWarehouse::activeKernelBlocks(std::set<subdomain_id_type> & set_buffer) const
{
  std::map<unsigned int, std::vector<Kernel *> >::const_iterator curr, end;
  end = _block_kernels.end();

  try
  {
    for (curr = _block_kernels.begin(); curr != end; ++curr)
      set_buffer.insert(subdomain_id_type(curr->first));
  }
  catch (std::exception &/*e*/)
  {
    mooseError("Invalid block specified in input file");
  }

  // return a boolean indicated whether there are any global kernels active
  return ! _active_kernels.empty();
}

void
KernelWarehouse::updateActiveKernels(Real t, Real dt)
{
  _active_kernels.clear();

  KernelIterator all_it = _all_kernels.begin();
  KernelIterator all_end = _all_kernels.end();

  for(; all_it != all_end; ++all_it)
    if((*all_it)->startTime() <= t + (1e-6 * dt) && (*all_it)->stopTime() >= t + (1e-6 * dt))
      _active_kernels.push_back(*all_it);


  _block_kernels.clear();

  std::map<unsigned int, std::vector<Kernel *> >::iterator block_it = _all_block_kernels.begin();
  std::map<unsigned int, std::vector<Kernel *> >::iterator block_end = _all_block_kernels.end();

  for(; block_it != block_end; ++block_it)
  {
    unsigned int block_num = block_it->first;
    _block_kernels[block_num].clear();

    KernelIterator all_block_it = block_it->second.begin();
    KernelIterator all_block_end = block_it->second.end();

    for(; all_block_it != all_block_end; ++all_block_it)
      if((*all_block_it)->startTime() <= t + (1e-6 * dt) && (*all_block_it)->stopTime() >= t + (1e-6 * dt))
        _block_kernels[block_num].push_back(*all_block_it);
  }
}
