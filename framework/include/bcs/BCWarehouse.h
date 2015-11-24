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
#include "MooseTypes.h"

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

  ///@{
  /**
   * Methods for adding the various types of BoundaryCondition objects to the warehouse
   * @param boundary_ids A set of BoundaryIDs that the supplied object pointer is active on
   */
  void addPresetNodalBC(const std::set<BoundaryID> & boundary_ids, MooseSharedPointer<PresetNodalBC> & bc);
  ///@}

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
  /// presetting nodal boundary condition on a boundary
  std::map<BoundaryID, std::vector<PresetNodalBC *> > _preset_nodal_bcs;
};

#endif // BCWAREHOUSE_H
