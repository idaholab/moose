#include "AuxWarehouse.h"

AuxWarehouse::AuxWarehouse()
{
}

AuxWarehouse::~AuxWarehouse()
{
  for(AuxKernelIterator j=_active_nodal_aux_kernels.begin(); j!=_active_nodal_aux_kernels.end(); ++j)
    delete *j;

  for(AuxKernelIterator j=_active_element_aux_kernels.begin(); j!=_active_element_aux_kernels.end(); ++j)
    delete *j;

  for(AuxKernelIterator j=_aux_bcs.begin(); j!=_aux_bcs.end(); ++j)
    delete *j;
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
AuxWarehouse::addBC(AuxKernel *aux)
{
  _aux_bcs.push_back(aux);
}

void
AuxWarehouse::addActiveBC(unsigned int boundary_id, AuxKernel *aux)
{
  _active_bcs[boundary_id].push_back(aux);
}

void
AuxWarehouse::addAuxKernel(AuxKernel *aux, std::set<subdomain_id_type> block_ids)
{
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

