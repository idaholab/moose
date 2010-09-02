/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BCWarehouse.h"

BCWarehouse::BCWarehouse()
{
}

BCWarehouse::~BCWarehouse()
{
  {
    std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator i;
    for(i=_active_bcs.begin(); i!=_active_bcs.end(); ++i)
    {
      BCIterator k;
      for(k=(i->second).begin(); k!=(i->second).end(); ++k)
      {
        delete *k;
      }
    }
  }

  {
    std::map<unsigned int, std::vector<BoundaryCondition *> >::iterator i;
    for(i=_active_nodal_bcs.begin(); i!=_active_nodal_bcs.end(); ++i)
    {
      BCIterator k;
      for(k=(i->second).begin(); k!=(i->second).end(); ++k)
      {
        delete *k;
      }
    }
  }
}

void
BCWarehouse::addBC(unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_bcs[boundary_id].push_back(bc);
}

void
BCWarehouse::addNodalBC(unsigned int boundary_id, BoundaryCondition *bc)
{
  _active_nodal_bcs[boundary_id].push_back(bc);
}

BCIterator
BCWarehouse::activeBCsBegin(unsigned int boundary_id)
{
  return _active_bcs[boundary_id].begin();
}

BCIterator
BCWarehouse::activeBCsEnd(unsigned int boundary_id)
{
  return _active_bcs[boundary_id].end();
}


BCIterator
BCWarehouse::activeNodalBCsBegin(unsigned int boundary_id)
{
  return _active_nodal_bcs[boundary_id].begin();
}

BCIterator
BCWarehouse::activeNodalBCsEnd(unsigned int boundary_id)
{
  return _active_nodal_bcs[boundary_id].end();
}

void
BCWarehouse::activeBoundaries(std::set<short> & set_buffer) const
{
  std::map<unsigned int, std::vector<BoundaryCondition *> >::const_iterator curr;

  for (curr = _active_bcs.begin(); curr != _active_bcs.end(); ++curr)
      set_buffer.insert(curr->first);

  for (curr = _active_nodal_bcs.begin(); curr != _active_nodal_bcs.end(); ++curr)
      set_buffer.insert(curr->first);
}
