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
#include "TimeDerivative.h"
#include "Kernel.h"
#include "ScalarKernel.h"

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
    if (dynamic_cast<TimeKernel *>(kernel) != NULL)
    {
      _time_global_kernels.push_back(kernel);
    }
    else
    {
      _nontime_global_kernels.push_back(kernel);
    }
  }
  else
  {
    for (std::set<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID blk_id = *it;
      if (dynamic_cast<TimeKernel *>(kernel) != NULL)
      {
        _time_block_kernels[blk_id].push_back(kernel);
      }
      else
      {
        _nt_block_kernels[blk_id].push_back(kernel);
      }
    }
  }
}

void
KernelWarehouse::addScalarKernel(ScalarKernel *kernel)
{
  _scalar_kernels.push_back(kernel);
}

void
KernelWarehouse::updateActiveKernels(unsigned int subdomain_id)
{
  _active_kernels.clear();
  _active_var_kernels.clear();
  _time_kernels.clear();
  _non_time_kernels.clear();
  // add kernels that live everywhere
  for (std::vector<Kernel *>::const_iterator it = _time_global_kernels.begin(); it != _time_global_kernels.end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->isActive())
    {
      _time_kernels.push_back(kernel);
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }
  for (std::vector<Kernel *>::const_iterator it = _nontime_global_kernels.begin(); it != _nontime_global_kernels.end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->isActive())
    {
      _non_time_kernels.push_back(kernel);
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }
  // then kernels that live on a specified block
  for (std::vector<Kernel *>::const_iterator it = _time_block_kernels[subdomain_id].begin(); it != _time_block_kernels[subdomain_id].end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->isActive())
    {
      _time_kernels.push_back(kernel);
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }

  for (std::vector<Kernel *>::const_iterator it = _nt_block_kernels[subdomain_id].begin(); it != _nt_block_kernels[subdomain_id].end(); ++it)
  {
    Kernel * kernel = *it;
    if (kernel->isActive())
    {
      _non_time_kernels.push_back(kernel);
      _active_kernels.push_back(kernel);
      _active_var_kernels[kernel->variable().number()].push_back(kernel);
    }
  }
}

bool
KernelWarehouse::subdomains_covered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const
{
  subdomains_covered.clear();
  unique_variables.clear();

  for (std::vector<Kernel *>::const_iterator it = _all_kernels.begin(); it != _all_kernels.end(); ++it)
    unique_variables.insert((*it)->variable().name());

  if (!_time_global_kernels.empty() || !_nontime_global_kernels.empty())
    return true;
  else
  {
    for (std::map<SubdomainID, std::vector<Kernel *> >::const_iterator it = _time_block_kernels.begin();
         it != _time_block_kernels.end(); ++it)
      subdomains_covered.insert(it->first);
    for (std::map<SubdomainID, std::vector<Kernel *> >::const_iterator it = _nt_block_kernels.begin();
         it != _nt_block_kernels.end(); ++it)
      subdomains_covered.insert(it->first);

    return false;
  }
}
