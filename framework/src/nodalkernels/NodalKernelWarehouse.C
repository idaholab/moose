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

#include "NodalKernelWarehouse.h"
#include "NodalKernel.h"


NodalKernelWarehouse::NodalKernelWarehouse()
{
}

NodalKernelWarehouse::~NodalKernelWarehouse()
{
}

void
NodalKernelWarehouse::initialSetup()
{
  for (std::vector<NodalKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
NodalKernelWarehouse::timestepSetup()
{
  for (std::vector<NodalKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
NodalKernelWarehouse::residualSetup()
{
  for (std::vector<NodalKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->residualSetup();
}

void
NodalKernelWarehouse::jacobianSetup()
{
  for (std::vector<NodalKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->jacobianSetup();
}

void
NodalKernelWarehouse::addNodalKernel(MooseSharedPointer<NodalKernel> & nodal_kernel)
{
  // Make certain that the NodalKernel is valid
  mooseAssert(nodal_kernel, "NodalKernel is NULL");

  // Add to elemental/nodal storage
  _all_nodal_kernels.push_back(nodal_kernel);

  // Get the SubdomainIDs for this object
  const std::set<SubdomainID> & block_ids(nodal_kernel->hasBlocks(Moose::ANY_BLOCK_ID) ? nodal_kernel->meshBlockIDs() : nodal_kernel->blockIDs()) ;

  // Populate the elemental and nodal block restricted maps
  for (std::set<SubdomainID>::const_iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    _active_block_nodal_kernels[*it].push_back(nodal_kernel);
}

bool
NodalKernelWarehouse::subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const
{
  for (std::vector<MooseSharedPointer<NodalKernel> >::const_iterator it = _all_nodal_kernels.begin(); it != _all_nodal_kernels.end(); ++it)
    unique_variables.insert((*it)->variable().name());

  for (std::map<SubdomainID, std::vector<MooseSharedPointer<NodalKernel> > >::const_iterator it = _active_block_nodal_kernels.begin();
       it != _active_block_nodal_kernels.end(); ++it)
    subdomains_covered.insert(it->first);

  return false;
}
