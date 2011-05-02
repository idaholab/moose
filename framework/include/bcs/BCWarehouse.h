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

#include "IntegratedBC.h"
#include "NodalBC.h"
#include "PresetNodalBC.h"

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

  void addBC(unsigned int boundary_id, IntegratedBC *bc);
  void addNodalBC(unsigned int boundary_id, NodalBC *bc);
  void addPresetNodalBC(unsigned int boundary_id, PresetNodalBC *bc);

  /**
   * Get boundary conditions on a specified boundary id
   */
  std::vector<IntegratedBC *> & getBCs(unsigned int boundary_id);
  /**
   * Get nodal boundary conditions on a specified boundary id
   */
  std::vector<NodalBC *> & getNodalBCs(unsigned int boundary_id);
  /**
   * Get presetting (;-)) nodal boundary conditions on a specified boundary id
   */
  std::vector<PresetNodalBC *> & getPresetNodalBCs(unsigned int boundary_id);

  /**
   * Get list of active boundaries
   * @param bnds[out] set of boundaries that are active
   */
  void activeBoundaries(std::set<short> & bnds) const;

protected:
  std::map<unsigned int, std::vector<IntegratedBC *> > _bcs;                    ///< integrated boundary conditions on a boundary
  std::map<unsigned int, std::vector<NodalBC *> > _nodal_bcs;                   ///< nodal boundary conditions on a boundary
  std::map<unsigned int, std::vector<PresetNodalBC *> > _preset_nodal_bcs;      ///< presetting nodal boundary condition on a boundary
};

#endif // BCWAREHOUSE_H
