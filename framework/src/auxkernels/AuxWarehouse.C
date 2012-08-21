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

#include "AuxWarehouse.h"
#include "AuxKernel.h"
#include "AuxScalarKernel.h"


AuxWarehouse::AuxWarehouse()
{
}

AuxWarehouse::~AuxWarehouse()
{
  for (std::vector<AuxKernel *>::const_iterator j = all().begin(); j != all().end(); ++j)
    delete *j;

  for (std::vector<AuxScalarKernel *>::const_iterator i = _scalar_kernels.begin(); i != _scalar_kernels.end(); ++i)
    delete *i;
}

void
AuxWarehouse::initialSetup()
{
  // Sort the auxKernels
  sortAuxKernels(_active_nodal_aux_kernels);
  sortAuxKernels(_active_element_aux_kernels);

  for (std::map<SubdomainID, std::vector<AuxKernel *> >::iterator i = _active_block_nodal_aux_kernels.begin();
       i != _active_block_nodal_aux_kernels.end(); ++i)
    sortAuxKernels(i->second);

  for (std::map<SubdomainID, std::vector<AuxKernel *> >::iterator i = _active_block_element_aux_kernels.begin();
       i != _active_block_element_aux_kernels.end(); ++i)
    sortAuxKernels(i->second);

  for (std::map<BoundaryID, std::vector<AuxKernel *> >::iterator i =  _active_nodal_bcs.begin();
       i != _active_nodal_bcs.end(); ++i)
    sortAuxKernels(i->second);

  for (std::vector<AuxKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->initialSetup();
}

void
AuxWarehouse::timestepSetup()
{
  for (std::vector<AuxKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->timestepSetup();
}

void
AuxWarehouse::residualSetup()
{
  for (std::vector<AuxKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->residualSetup();
}

void
AuxWarehouse::jacobianSetup()
{
  for (std::vector<AuxKernel *>::const_iterator i = all().begin(); i != all().end(); ++i)
    (*i)->jacobianSetup();
}

void
AuxWarehouse::addActiveBC(BoundaryID boundary_id, AuxKernel *aux)
{
  _all_aux_kernels.push_back(aux);
  if (aux->isNodal())
  {
    _active_nodal_bcs[boundary_id].push_back(aux);
  }
  else
  {
    _all_elem_bcs.push_back(aux);
    _elem_bcs[boundary_id].push_back(aux);
  }
}

void
AuxWarehouse::addAuxKernel(AuxKernel *aux, std::set<SubdomainID> block_ids)
{
  _all_aux_kernels.push_back(aux);
  if (block_ids.empty())
  {
    if(aux->isNodal())
    {
      _all_nodal_aux_kernels.push_back(aux);
      _active_nodal_aux_kernels.push_back(aux);
    }
    else
    {
      _all_element_aux_kernels.push_back(aux);
      _active_element_aux_kernels.push_back(aux);
    }
  }
  else
  {
    for(std::set<SubdomainID>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      SubdomainID id = *it;

      if(aux->isNodal())
      {
        _all_nodal_aux_kernels.push_back(aux);
        _active_block_nodal_aux_kernels[id].push_back(aux);
      }
      else
      {
        _all_element_aux_kernels.push_back(aux);
        _active_block_element_aux_kernels[id].push_back(aux);
      }
    }
  }
}

void
AuxWarehouse::addScalarKernel(AuxScalarKernel *kernel)
{
  _scalar_kernels.push_back(kernel);
}

void
AuxWarehouse::sortAuxKernels(std::vector<AuxKernel *> & aux_vector)
{
  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort(aux_vector.begin(), aux_vector.end());
  }
  catch(CyclicDependencyException<DependencyResolverInterface *> & e)
  {
    std::ostringstream oss;

    oss << "Cyclic dependency detected in aux kernel ordering:\n";
    const std::multimap<DependencyResolverInterface *, DependencyResolverInterface *> & depends = e.getCyclicDependencies();
    for (std::multimap<DependencyResolverInterface *, DependencyResolverInterface *>::const_iterator it = depends.begin(); it != depends.end(); ++it)
      oss << (static_cast<AuxKernel *>(it->first))->name() << " -> " << (static_cast<AuxKernel *>(it->second))->name() << "\n";
    mooseError(oss.str());
  }
}
