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

#include "Warehouse.h"

#include <vector>
#include <map>
#include <string>
#include <list>
#include <set>

class AuxKernel;
class AuxScalarKernel;

/**
 * Warehouse for storing auxiliary kernels
 *
 * Used inside auxiliary system to store aux. kernels
 */
class AuxWarehouse : public Warehouse<AuxKernel>
{
public:
  AuxWarehouse();
  virtual ~AuxWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  const std::vector<AuxKernel *> & allElementKernels() const { return _all_element_aux_kernels; }
  const std::vector<AuxKernel *> & allNodalKernels() const { return _all_nodal_aux_kernels; }

  const std::vector<AuxKernel *> & activeBlockNodalKernels(SubdomainID block) { return _active_block_nodal_aux_kernels[block]; }
  const std::vector<AuxKernel *> & activeBlockElementKernels(SubdomainID block) { return _active_block_element_aux_kernels[block]; }

  const std::vector<AuxKernel *> & activeBCs(BoundaryID boundary_id) { return _active_nodal_bcs[boundary_id]; }
  const std::vector<AuxKernel *> & allElementalBCs() { return _all_elem_bcs; }
  const std::vector<AuxKernel *> & elementalBCs(BoundaryID boundary_id) { return _elem_bcs[boundary_id]; }

  /**
   * Get list of scalar kernels
   * @return The list of scalar active kernels
   */
  const std::vector<AuxScalarKernel *> & scalars() { return _scalar_kernels; }

  /**
   * Adds an auxiliary kernel
   * @param aux Kernel being added
   * @param block_ids Set of subdomain this kernel is active on
   */
  void addAuxKernel(MooseSharedPointer<AuxKernel> & aux);

  /**
   * Add a scalar kernels
   * @param kernel Scalar kernel being added
   */
  void addScalarKernel(MooseSharedPointer<AuxScalarKernel> & kernel);

protected:
  /// all element aux kernels
  std::vector<AuxKernel *> _all_element_aux_kernels;
  /// all nodal aux kernels
  std::vector<AuxKernel *> _all_nodal_aux_kernels;

  /// nodal kernels active on a block
  std::map<SubdomainID, std::vector<AuxKernel *> > _active_block_nodal_aux_kernels;
  /// elemental kernels active on a block
  std::map<SubdomainID, std::vector<AuxKernel *> > _active_block_element_aux_kernels;

  /// nodal aux boundary conditions
  std::map<BoundaryID, std::vector<AuxKernel *> > _active_nodal_bcs;
  /// All elemental BCs
  std::vector<AuxKernel *> _all_elem_bcs;
  /// Elemental auxs at boundaries
  std::map<BoundaryID, std::vector<AuxKernel *> > _elem_bcs;

  /// Scalar kernels
  std::vector<AuxScalarKernel *> _scalar_kernels;

private:
  ///@{
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<AuxKernel> > _all_ptrs;
  std::vector<MooseSharedPointer<AuxScalarKernel> > _all_scalar_ptrs;
  ///@}

  /**
   * This routine uses the Dependency Resolver to sort AuxKernels based on dependencies they
   * might have on coupled values
   */
  void sortAuxKernels(std::vector<AuxKernel *> & aux_vector);
};

#endif // AUXWAREHOUSE_H
