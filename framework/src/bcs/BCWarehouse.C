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
#include "IntegratedBC.h"
#include "NodalBC.h"
#include "PresetNodalBC.h"

BCWarehouse::BCWarehouse() :
    Warehouse<BoundaryCondition>()
{
}

BCWarehouse::~BCWarehouse()
{
}

void
BCWarehouse::initialSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();

  for (std::map<BoundaryID, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->initialSetup();
}

void
BCWarehouse::timestepSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();

  for (std::map<BoundaryID, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
BCWarehouse::residualSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();

  for (std::map<BoundaryID, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
BCWarehouse::jacobianSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();

  for (std::map<BoundaryID, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
}

void
BCWarehouse::addBC(BoundaryID boundary_id, MooseSharedPointer<IntegratedBC> & bc)
{
  _all_objects.push_back(bc.get());
  _all_ptrs.push_back(MooseSharedNamespace::static_pointer_cast<BoundaryCondition>(bc));
  _bcs[boundary_id].push_back(bc.get());
}

void
BCWarehouse::addNodalBC(BoundaryID boundary_id, MooseSharedPointer<NodalBC> & bc)
{
  _all_objects.push_back(bc.get());
  _all_ptrs.push_back(MooseSharedNamespace::static_pointer_cast<BoundaryCondition>(bc));
  _nodal_bcs[boundary_id].push_back(bc.get());
}

void
BCWarehouse::addPresetNodalBC(BoundaryID boundary_id, MooseSharedPointer<PresetNodalBC> & bc)
{
  _all_objects.push_back(bc.get());
  _all_ptrs.push_back(MooseSharedNamespace::static_pointer_cast<BoundaryCondition>(bc));
  _preset_nodal_bcs[boundary_id].push_back(bc.get());
}

const std::vector<IntegratedBC *> &
BCWarehouse::getBCs(BoundaryID boundary_id) const
{
  mooseAssert(_bcs.find(boundary_id) != _bcs.end(), "Unknown boundary id: " << boundary_id);
  return _bcs.find(boundary_id)->second;
}

const std::vector<NodalBC *> &
BCWarehouse::getNodalBCs(BoundaryID boundary_id) const
{
  mooseAssert(_nodal_bcs.find(boundary_id) != _nodal_bcs.end(), "Unknown boundary id: " << boundary_id);
  return _nodal_bcs.find(boundary_id)->second;
}

const std::vector<PresetNodalBC *> &
BCWarehouse::getPresetNodalBCs(BoundaryID boundary_id) const
{
  mooseAssert(_preset_nodal_bcs.find(boundary_id) != _preset_nodal_bcs.end(), "Unknown boundary id: " << boundary_id);
  return _preset_nodal_bcs.find(boundary_id)->second;
}

void
BCWarehouse::activeBoundaries(std::set<BoundaryID> & bnds) const
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    bnds.insert(curr->first);

  for (std::map<BoundaryID, std::vector<NodalBC *> >::const_iterator curr = _nodal_bcs.begin(); curr != _nodal_bcs.end(); ++curr)
    bnds.insert(curr->first);

  for (std::map<BoundaryID, std::vector<PresetNodalBC *> >::const_iterator curr = _preset_nodal_bcs.begin(); curr != _preset_nodal_bcs.end(); ++curr)
    bnds.insert(curr->first);
}

void
BCWarehouse::activeIntegrated(BoundaryID boundary_id, std::vector<IntegratedBC *> & active_integrated) const
{
  active_integrated.clear();

  if (_bcs.find(boundary_id) != _bcs.end())
    for (std::vector<IntegratedBC *>::const_iterator it = _bcs.at(boundary_id).begin(); it != _bcs.at(boundary_id).end(); ++it)
      if ((*it)->isActive())
        active_integrated.push_back(*it);
}

void
BCWarehouse::activeNodal(BoundaryID boundary_id, std::vector<NodalBC *> & active_nodal) const
{
  active_nodal.clear();

  if (_nodal_bcs.find(boundary_id) != _nodal_bcs.end())
    for (std::vector<NodalBC *>::const_iterator it = _nodal_bcs.at(boundary_id).begin(); it != _nodal_bcs.at(boundary_id).end(); ++it)
      if ((*it)->isActive())
        active_nodal.push_back(*it);
}

void
BCWarehouse::activePresetNodal(BoundaryID boundary_id, std::vector<PresetNodalBC *> & active_preset) const
{
  active_preset.clear();

  if (_preset_nodal_bcs.find(boundary_id) != _preset_nodal_bcs.end())
    for (std::vector<PresetNodalBC *>::const_iterator it = _preset_nodal_bcs.at(boundary_id).begin(); it != _preset_nodal_bcs.at(boundary_id).end(); ++it)
      if ((*it)->isActive())
        active_preset.push_back(*it);
}
