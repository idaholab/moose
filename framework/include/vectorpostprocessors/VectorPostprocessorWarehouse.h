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

#ifndef VECTORPOSTPROCESSORWAREHOUSE_H
#define VECTORPOSTPROCESSORWAREHOUSE_H

// MOOSE includes
#include "Warehouse.h"
#include "MooseTypes.h"

// Forward declarations
class VectorPostprocessor;
class ElementVectorPostprocessor;
class InternalSideVectorPostprocessor;
class NodalVectorPostprocessor;
class SideVectorPostprocessor;
class GeneralVectorPostprocessor;

/**
 * Holds VectorPostprocessors and provides some services
 */
class VectorPostprocessorWarehouse : public Warehouse<VectorPostprocessor>
{
public:
  VectorPostprocessorWarehouse();
  virtual ~VectorPostprocessorWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Get the list of all  elemental VectorPostprocessors
   * @param block_id Block ID
   * @return The list of all elemental VectorPostprocessors
   */
  const std::vector<ElementVectorPostprocessor *> & elementVectorPostprocessors(SubdomainID block_id) const;

  /**
   * Get the list of side VectorPostprocessors
   * @param boundary_id Boundary ID
   * @return The list of side VectorPostprocessors
   */
  const std::vector<SideVectorPostprocessor *> & sideVectorPostprocessors(BoundaryID boundary_id) const;

  /**
   * Get the list of nodal VectorPostprocessors
   * @param boundary_id Boundary ID
   * @return The list of all nodal VectorPostprocessors
   */
  const std::vector<NodalVectorPostprocessor *> & nodalVectorPostprocessors(BoundaryID boundary_id) const;

  /**
   * Get the list of nodal VectorPostprocessors restricted on the specified subdomain
   * @param subdomain_id Subdomain ID
   * @return The list of all block nodal VectorPostprocessors
   */
  const std::vector<NodalVectorPostprocessor *> & blockNodalVectorPostprocessors(SubdomainID subdomain_id) const;

  /**
   * Get the list general VectorPostprocessors
   * @return The list of general VectorPostprocessors
   */
  const std::vector<GeneralVectorPostprocessor *> & genericVectorPostprocessors() const { return _generic_VectorPostprocessors; }

  /**
   * Get a pointer to a VectorPostprocessor
   * @param name The name of the VectorPostprocessor to retrieve
   * @return A VectorPostprocessor for the given name
   */
  VectorPostprocessor * getVectorPostprocessor(std::string name);

  /**
   * Add a VectorPostprocessor
   * @param vector_postprocessor VectorPostprocessor being added
   */
  void addVectorPostprocessor(MooseSharedPointer<VectorPostprocessor> vector_postprocessor);

  /**
   * Get the list of blocks with VectorPostprocessors
   * @return The list of block IDs with VectorPostprocessors
   */
  const std::set<SubdomainID> & blocks() const { return _block_ids_with_VectorPostprocessors; }

  /**
   * Get the list of boundary IDs with VectorPostprocessors
   * @return The list of boundary IDs with VectorPostprocessors
   */
  const std::set<BoundaryID> & boundaryIds() const { return _boundary_ids_with_VectorPostprocessors; }

  /**
   * Get the list of nodeset IDs with VectorPostprocessors
   * @return The list of nodeset IDs with VectorPostprocessors
   */
  const std::set<BoundaryID> & nodesetIds() const { return _nodeset_ids_with_VectorPostprocessors; }

  /**
   * Get the list of subdomain IDs with *nodal* VectorPostprocessors
   * @return The list of subdomain IDs with VectorPostprocessors
   */
  const std::set<SubdomainID> & blockNodalIds() const { return _block_ids_with_nodal_VectorPostprocessors; }

protected:
  std::vector<ElementVectorPostprocessor *> _all_element_VectorPostprocessors;
  std::vector<NodalVectorPostprocessor *> _all_nodal_VectorPostprocessors;
  std::vector<SideVectorPostprocessor *> _all_side_VectorPostprocessors;
  std::vector<InternalSideVectorPostprocessor *> _all_internal_side_VectorPostprocessors;
  std::vector<GeneralVectorPostprocessor *> _all_generic_VectorPostprocessors;

  std::map<SubdomainID, std::vector<ElementVectorPostprocessor *> > _element_VectorPostprocessors;
  std::map<SubdomainID, std::vector<InternalSideVectorPostprocessor *> > _internal_side_VectorPostprocessors;
  std::map<BoundaryID, std::vector<SideVectorPostprocessor *> > _side_VectorPostprocessors;
  std::map<BoundaryID, std::vector<NodalVectorPostprocessor *> > _nodal_VectorPostprocessors;
  // Block restricted nodal pps
  std::map<SubdomainID, std::vector<NodalVectorPostprocessor *> > _block_nodal_VectorPostprocessors;

  std::vector<GeneralVectorPostprocessor *> _generic_VectorPostprocessors;

  /// All of the block ids that have VectorPostprocessors specified to act on them
  std::set<SubdomainID> _block_ids_with_VectorPostprocessors;
  /// All of the boundary ids that have VectorPostprocessors specified to act on them
  std::set<BoundaryID> _boundary_ids_with_VectorPostprocessors;
  /// All of the nodeset ids that have VectorPostprocessors specified to act on them
  std::set<BoundaryID> _nodeset_ids_with_VectorPostprocessors;
  /// All of the block ids that have nodal VectorPostprocessors specified to act on them
  std::set<SubdomainID> _block_ids_with_nodal_VectorPostprocessors;

private:
  /// Hold shared pointers for automatic cleanup
  std::vector<MooseSharedPointer<VectorPostprocessor> > _all_ptrs;
};

#endif // VECTORPOSTPROCESSORWAREHOUSE_H
