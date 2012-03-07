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
  for (std::vector<AuxKernel *>::const_iterator j = all().begin(); j != all().end(); ++j)
    delete *j;

  for (std::vector<AuxScalarKernel *>::const_iterator i = _scalar_kernels.begin(); i != _scalar_kernels.end(); ++i)
    delete *i;
}

void
AuxWarehouse::initialSetup()
{
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
    for(std::set<subdomain_id_type>::iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      subdomain_id_type id = *it;

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
