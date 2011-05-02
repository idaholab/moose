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

#ifndef AUXWAREHOUSE_H
#define AUXWAREHOUSE_H

#include <vector>
#include <map>
#include <string>
#include <list>

#include "AuxKernel.h"


/**
 * Warehouse for storing auxiliary kernels
 *
 * Used inside auxiliary system to store aux. kernels
 */
class AuxWarehouse
{
public:
  AuxWarehouse();
  virtual ~AuxWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  const std::vector<AuxKernel *> & all() { return _all_aux_kernels; }

  const std::vector<AuxKernel *> & activeNodalKernels() { return _active_nodal_aux_kernels; }
  const std::vector<AuxKernel *> & activeElementKernels() { return _active_element_aux_kernels; }

  const std::vector<AuxKernel *> & activeBlockNodalKernels(unsigned int block) { return _active_block_nodal_aux_kernels[block]; }
  const std::vector<AuxKernel *> & activeBlockElementKernels(unsigned int block) { return _active_block_element_aux_kernels[block]; }

  const std::vector<AuxKernel *> & activeBCs(unsigned int boundary_id) { return _active_bcs[boundary_id]; }

  /**
   * Adds a boundary condition aux kernel
   * @param boundary_id Boundary ID this kernel works on
   * @param aux BC kernel being added
   */
  void addActiveBC(unsigned int boundary_id, AuxKernel *aux);

  /**
   * Adds an auxiliary kernel
   * @param aux Kernel being added
   * @param block_ids Set of subdomain this kernel is active on
   */
  void addAuxKernel(AuxKernel *aux, std::set<subdomain_id_type> block_ids);

protected:
  std::vector<AuxKernel *> _all_aux_kernels;                                                    ///< all aux kernels (kernels and BCs)

  std::vector<AuxKernel *> _active_nodal_aux_kernels;                                           ///< nodal kernels active everywhere
  std::vector<AuxKernel *> _active_element_aux_kernels;                                         ///< elemental kernels active everywhere
  std::map<unsigned int, std::vector<AuxKernel *> > _active_block_nodal_aux_kernels;            ///< nodal kernels active on a block
  std::map<unsigned int, std::vector<AuxKernel *> > _active_block_element_aux_kernels;          ///< elemental kernels active on a block

  std::map<unsigned int, std::vector<AuxKernel *> > _active_bcs;                                ///< aux boundary conditions
};

#endif // AUXWAREHOUSE_H
