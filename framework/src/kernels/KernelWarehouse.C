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

KernelWarehouse::KernelWarehouse()
{
}

KernelWarehouse::~KernelWarehouse()
{
  for (std::vector<Kernel *>::const_iterator i = _all_kernels.begin(); i != _all_kernels.end(); ++i)
    delete *i;

  for (std::vector<ScalarKernel *>::const_iterator i = _scalar_kernels.begin(); i != _scalar_kernels.end(); ++i)
    delete *i;
}

void
KernelWarehouse::initialSetup()
{
  for (std::vector<Kernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
KernelWarehouse::timestepSetup()
{
  for (std::vector<Kernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
KernelWarehouse::residualSetup()
{
  for (std::vector<Kernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->residualSetup();
}

void
KernelWarehouse::jacobianSetup()
{
  for (std::vector<Kernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->jacobianSetup();
}

void
KernelWarehouse::addKernel(Kernel *kernel, const std::set<SubdomainID> & block_ids)
{
  _all_kernels.push_back(kernel);

  if (block_ids.empty())
  {
    _global_kernels.push_back(kernel);
  }
  else
  {
    for (std::set<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;
      _block_kernels[blk_id].push_back(kernel);
    }
  }
}

void
KernelWarehouse::addScalarKernel(ScalarKernel *kernel)
{
  _scalar_kernels.push_back(kernel);
}

void
KernelWarehouse::updateActiveKernels(Real t, Real dt, unsigned int subdomain_id)
{
  _active_kernels.clear();
  _active_var_kernels.clear();

  // add kernels that live everywhere
  for (std::vector<Kernel *>::const_iterator it = _global_kernels.begin(); it != _global_kernels.end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->startTime() <= t + (1e-6 * dt) && kernel->stopTime() >= t + (1e-6 * dt))
    {
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }

  // then kernels that live on a specified block
  for (std::vector<Kernel *>::const_iterator it = _block_kernels[subdomain_id].begin(); it != _block_kernels[subdomain_id].end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->startTime() <= t + (1e-6 * dt) && kernel->stopTime() >= t + (1e-6 * dt))
    {
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }
}

bool
KernelWarehouse::subdomains_covered(std::set<SubdomainID> & return_set) const
{
  return_set.clear();

  if (!_global_kernels.empty())
    return true;
  else
  {
    for (std::map<SubdomainID, std::vector<Kernel *> >::const_iterator it = _block_kernels.begin();
         it != _block_kernels.end(); ++it)
      return_set.insert(it->first);
    return false;
  }
}
