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

#include "BCWarehouse.h"

BCWarehouse::BCWarehouse()
{
}

BCWarehouse::~BCWarehouse()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::iterator i = _bcs.begin(); i != _bcs.end(); ++i)
    for (std::vector<IntegratedBC *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;

  for (std::map<unsigned int, std::vector<NodalBC *> >::iterator i = _nodal_bcs.begin(); i != _nodal_bcs.end(); ++i)
    for (std::vector<NodalBC *>::iterator k=(i->second).begin(); k!=(i->second).end(); ++k)
      delete *k;
}

void
BCWarehouse::initialSetup()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();
}

void
BCWarehouse::timestepSetup()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
BCWarehouse::residualSetup()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
BCWarehouse::jacobianSetup()
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for(unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
}

void
BCWarehouse::addBC(unsigned int boundary_id, IntegratedBC *bc)
{
  _bcs[boundary_id].push_back(bc);
}

void
BCWarehouse::addNodalBC(unsigned int boundary_id, NodalBC *bc)
{
  _nodal_bcs[boundary_id].push_back(bc);
}

void
BCWarehouse::addPresetNodalBC(unsigned int boundary_id, PresetNodalBC *bc)
{
  _preset_nodal_bcs[boundary_id].push_back(bc);
}

std::vector<IntegratedBC *> &
BCWarehouse::getBCs(unsigned int boundary_id)
{
  return _bcs[boundary_id];
}

std::vector<NodalBC *> &
BCWarehouse::getNodalBCs(unsigned int boundary_id)
{
  return _nodal_bcs[boundary_id];
}

std::vector<PresetNodalBC *> &
BCWarehouse::getPresetNodalBCs(unsigned int boundary_id)
{
  return _preset_nodal_bcs[boundary_id];
}

void
BCWarehouse::activeBoundaries(std::set<short> & bnds) const
{
  for (std::map<unsigned int, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    bnds.insert(curr->first);

  for (std::map<unsigned int, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    bnds.insert(curr->first);

  for (std::map<unsigned int, std::vector<PresetNodalBC *> >::const_iterator curr = _preset_nodal_bcs.begin(); curr != _preset_nodal_bcs.end(); ++curr)
    bnds.insert(curr->first);
}
