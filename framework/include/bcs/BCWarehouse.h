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

#ifndef BCWAREHOUSE_H
#define BCWAREHOUSE_H

#include <map>
#include <vector>
#include <set>
#include "Warehouse.h"

class BoundaryCondition;
class IntegratedBC;
class NodalBC;
class PresetNodalBC;

/**
 * Warehouse for storing boundary conditions (for non-linear variables)
 */
class BCWarehouse : public Warehouse<BoundaryCondition>
{
public:
  BCWarehouse();
  virtual ~BCWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  void addBC(BoundaryID boundary_id, MooseSharedPointer<IntegratedBC> & bc);
  void addNodalBC(BoundaryID boundary_id, MooseSharedPointer<NodalBC> & bc);
  void addPresetNodalBC(BoundaryID boundary_id, MooseSharedPointer<PresetNodalBC> & bc);

  /**
   * Get boundary conditions on a specified boundary id
   */
  const std::vector<IntegratedBC *> & getBCs(BoundaryID boundary_id) const;

  /**
   * Check for the existence of nodal bcs for the specified boundary
   */
  bool hasNodalBCs(BoundaryID boundary_id) const { return _nodal_bcs.find(boundary_id) != _nodal_bcs.end(); }

  /**
   * Get nodal boundary conditions on a specified boundary id
   */
  const std::vector<NodalBC *> & getNodalBCs(BoundaryID boundary_id) const;

  /**
   * Get presetting (;-)) nodal boundary conditions on a specified boundary id
   */
  const std::vector<PresetNodalBC *> & getPresetNodalBCs(BoundaryID boundary_id) const;

  /**
   * Get list of active boundaries
   * @param bnds set of boundaries that are active
   */
  void activeBoundaries(std::set<BoundaryID> & bnds) const;

  /**
   * Get active integrated boundary conditions
   * @param boundary_id Boundary ID
   * @param active_integrated A vector to populate with the active integrated bcs
   */
  void activeIntegrated(BoundaryID boundary_id, std::vector<IntegratedBC *> & active_integrated) const;

  /**
   * Get active nodal boundary conditions
   * @param boundary_id Boundary ID
   * @param active_nodal A vector to populate with the active nodal bcs
   */
  void activeNodal(BoundaryID boundary_id, std::vector<NodalBC *> & active_nodal) const;

  /**
   * Get active preset nodal boundary conditions
   * @param boundary_id Boundary ID
   * @param active_preset A vector to populate with the active preset bcs
   */
  void activePresetNodal(BoundaryID boundary_id, std::vector<PresetNodalBC *> & active_preset) const;

protected:
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<BoundaryCondition> > _all_ptrs;

  /// integrated boundary conditions on a boundary
  std::map<BoundaryID, std::vector<IntegratedBC *> > _bcs;
  /// nodal boundary conditions on a boundary
  std::map<BoundaryID, std::vector<NodalBC *> > _nodal_bcs;
  /// presetting nodal boundary condition on a boundary
  std::map<BoundaryID, std::vector<PresetNodalBC *> > _preset_nodal_bcs;
};

#endif // BCWAREHOUSE_H
