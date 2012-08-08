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
#include "Moose.h"

class IntegratedBC;
class NodalBC;
class PresetNodalBC;

/**
 * Warehouse for storing boundary conditions (for non-linear variables)
 */
class BCWarehouse
{
public:
  BCWarehouse();
  virtual ~BCWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  void addBC(BoundaryID boundary_id, IntegratedBC *bc);
  void addNodalBC(BoundaryID boundary_id, NodalBC *bc);
  void addPresetNodalBC(BoundaryID boundary_id, PresetNodalBC *bc);

  /**
   * Get boundary conditions on a specified boundary id
   */
  std::vector<IntegratedBC *> & getBCs(BoundaryID boundary_id);
  /**
   * Get nodal boundary conditions on a specified boundary id
   */
  std::vector<NodalBC *> & getNodalBCs(BoundaryID boundary_id);
  /**
   * Get presetting (;-)) nodal boundary conditions on a specified boundary id
   */
  std::vector<PresetNodalBC *> & getPresetNodalBCs(BoundaryID boundary_id);

  /**
   * Get list of active boundaries
   * @param bnds[out] set of boundaries that are active
   */
  void activeBoundaries(std::set<BoundaryID> & bnds) const;

  /**
   * Get active integrated boundary conditions
   * @param boundary_id Boundary ID
   * @return Set of active integrated BCs
   */
  std::vector<IntegratedBC *> activeIntegrated(BoundaryID boundary_id);

  /**
   * Get active nodal boundary conditions
   * @param boundary_id Boundary ID
   * @return Set of active nodal BCs
   */
  std::vector<NodalBC *> activeNodal(BoundaryID boundary_id);

  /**
   * Get active preset nodal boundary conditions
   * @param boundary_id Boundary ID
   * @return Set of active preset nodal BCs
   */
  std::vector<PresetNodalBC *> activePresetNodal(BoundaryID boundary_id);

protected:
  /// integrated boundary conditions on a boundary
  std::map<BoundaryID, std::vector<IntegratedBC *> > _bcs;
  /// nodal boundary conditions on a boundary
  std::map<BoundaryID, std::vector<NodalBC *> > _nodal_bcs;
  /// presetting nodal boundary condition on a boundary
  std::map<BoundaryID, std::vector<PresetNodalBC *> > _preset_nodal_bcs;
};

#endif // BCWAREHOUSE_H
