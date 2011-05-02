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

AuxWarehouse::AuxWarehouse()
{
}

AuxWarehouse::~AuxWarehouse()
{
  for(AuxKernelIterator j=_all_aux_kernels.begin(); j!=_all_aux_kernels.end(); ++j)
    delete *j;
}

void
AuxWarehouse::initialSetup()
{
  for(AuxKernelIterator i=allAuxKernelsBegin(); i!=allAuxKernelsEnd(); ++i)
    (*i)->initialSetup();
}

void
AuxWarehouse::timestepSetup()
{
  for(AuxKernelIterator i=allAuxKernelsBegin(); i!=allAuxKernelsEnd(); ++i)
    (*i)->timestepSetup();
}

void
AuxWarehouse::residualSetup()
{
  for(AuxKernelIterator i=allAuxKernelsBegin(); i!=allAuxKernelsEnd(); ++i)
    (*i)->residualSetup();
}

void
AuxWarehouse::jacobianSetup()
{
  for(AuxKernelIterator i=allAuxKernelsBegin(); i!=allAuxKernelsEnd(); ++i)
    (*i)->jacobianSetup();
}

AuxKernelIterator
AuxWarehouse::activeNodalAuxKernelsBegin()
{
  return _active_nodal_aux_kernels.begin();
}

AuxKernelIterator
AuxWarehouse::activeNodalAuxKernelsEnd()
{
  return _active_nodal_aux_kernels.end();
}

AuxKernelIterator
AuxWarehouse::activeElementAuxKernelsBegin()
{
  return _active_element_aux_kernels.begin();
}

AuxKernelIterator
AuxWarehouse::activeElementAuxKernelsEnd()
{
  return _active_element_aux_kernels.end();
}

AuxKernelIterator
AuxWarehouse::activeAuxBCsBegin(unsigned int boundary_id)
{
  return _active_bcs[boundary_id].begin();
}

AuxKernelIterator
AuxWarehouse::activeAuxBCsEnd(unsigned int boundary_id)
{
  return _active_bcs[boundary_id].end();
}

AuxKernelIterator
AuxWarehouse::activeBlockNodalAuxKernelsBegin(unsigned int block)
{
  return _active_block_nodal_aux_kernels[block].begin();
}

AuxKernelIterator
AuxWarehouse::activeBlockNodalAuxKernelsEnd(unsigned int block)
{
  return _active_block_nodal_aux_kernels[block].end();
}

AuxKernelIterator
AuxWarehouse::activeBlockElementAuxKernelsBegin(unsigned int block)
{
  return _active_block_element_aux_kernels[block].begin();
}

AuxKernelIterator
AuxWarehouse::activeBlockElementAuxKernelsEnd(unsigned int block)
{
  return _active_block_element_aux_kernels[block].end();
}

std::list<AuxKernel *>
AuxWarehouse::getActiveNodalKernels()
{
  return std::list<AuxKernel *>(_active_nodal_aux_kernels.begin(), _active_nodal_aux_kernels.end());
}

std::list<AuxKernel *>
AuxWarehouse::getActiveElementKernels()
{
  return std::list<AuxKernel *>(_active_element_aux_kernels.begin(), _active_element_aux_kernels.end());
}

void
AuxWarehouse::setActiveNodalKernels(std::list<AuxKernel *> &auxs)
{
  _active_nodal_aux_kernels.assign(auxs.begin(), auxs.end());
}

void
AuxWarehouse::setActiveElementKernels(std::list<AuxKernel *> &auxs)
{
  _active_element_aux_kernels.assign(auxs.begin(), auxs.end());
}

void
AuxWarehouse::addActiveBC(unsigned int boundary_id, AuxKernel *aux)
{
  _all_aux_kernels.push_back(aux);
  _active_bcs[boundary_id].push_back(aux);
}

void
AuxWarehouse::addAuxKernel(AuxKernel *aux, std::set<subdomain_id_type> block_ids)
{
  _all_aux_kernels.push_back(aux);
  if (block_ids.empty())
  {
    if(aux->isNodal())
      _active_nodal_aux_kernels.push_back(aux);
    else
      _active_element_aux_kernels.push_back(aux);
  }
  else
  {
    for(std::set<subdomain_id_type>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      subdomain_id_type id = *it;

      if(aux->isNodal())
        _active_block_nodal_aux_kernels[id].push_back(aux);
      else
        _active_block_element_aux_kernels[id].push_back(aux);
    }
  }
}

AuxKernelIterator
AuxWarehouse::allAuxKernelsBegin()
{
  return _all_aux_kernels.begin();
}

AuxKernelIterator
AuxWarehouse::allAuxKernelsEnd()
{
  return _all_aux_kernels.end();
}
