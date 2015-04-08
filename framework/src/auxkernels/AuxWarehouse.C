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
}

void
AuxWarehouse::initialSetup()
{
  // Sort the auxKernels
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
AuxWarehouse::addAuxKernel(MooseSharedPointer<AuxKernel> & aux)
{
  // Make certain that the AuxKernel is valid
  mooseAssert(aux, "Auxkernel is NULL");

  // Add the pointer to the complete and the nodal or elemental lists
  _all_ptrs.push_back(aux);
  _all_objects.push_back(aux.get());

  // Boundary restricted
  if (aux->boundaryRestricted())
  {
    // Add to elemental boundary storage
    if (!aux->isNodal())
      _all_elem_bcs.push_back(aux.get());

    // Populate the elemental and nodal boundary restricted maps
    const std::set<BoundaryID> & boundary_ids = aux->boundaryIDs();
    for (std::set<BoundaryID>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
    {
      if (aux->isNodal())
        _active_nodal_bcs[*it].push_back(aux.get());
      else
        _elem_bcs[*it].push_back(aux.get());
    }
  }

  // Block restricted
  else
  {
    // Add to elemental/nodal storage
    if (aux->isNodal())
      _all_nodal_aux_kernels.push_back(aux.get());
    else
      _all_element_aux_kernels.push_back(aux.get());

    // Get the SubdomainIDs for this object
    const std::set<SubdomainID> & block_ids(aux->hasBlocks(Moose::ANY_BLOCK_ID) ? aux->meshBlockIDs() : aux->blockIDs()) ;

    // Populate the elemental and nodal block restricted maps
    for (std::set<SubdomainID>::const_iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      if (aux->isNodal())
        _active_block_nodal_aux_kernels[*it].push_back(aux.get());
      else
        _active_block_element_aux_kernels[*it].push_back(aux.get());
    }
  }
}

void
AuxWarehouse::addScalarKernel(MooseSharedPointer<AuxScalarKernel> & kernel)
{
  mooseAssert(kernel, "ScalarAuxkernel is NULL");

  _all_scalar_ptrs.push_back(kernel);
  _scalar_kernels.push_back(kernel.get());
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
