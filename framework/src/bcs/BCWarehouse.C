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
}

void
BCWarehouse::timestepSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->timestepSetup();
}

void
BCWarehouse::residualSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->residualSetup();
}

void
BCWarehouse::jacobianSetup()
{
  for (std::map<BoundaryID, std::vector<IntegratedBC *> >::const_iterator curr = _bcs.begin(); curr != _bcs.end(); ++curr)
    for (unsigned int i=0; i<curr->second.size(); i++)
      (curr->second)[i]->jacobianSetup();
}

void
  BCWarehouse::addPresetNodalBC(const std::set<BoundaryID> & boundary_ids, MooseSharedPointer<PresetNodalBC> & bc)
{
  _all_objects.push_back(bc.get());
  _all_ptrs.push_back(MooseSharedNamespace::static_pointer_cast<BoundaryCondition>(bc));

  for (std::set<BoundaryID>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
    _preset_nodal_bcs[*it].push_back(bc.get());
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

  for (std::map<BoundaryID, std::vector<PresetNodalBC *> >::const_iterator curr = _preset_nodal_bcs.begin(); curr != _preset_nodal_bcs.end(); ++curr)
    bnds.insert(curr->first);
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
