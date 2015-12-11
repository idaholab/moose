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

#ifndef POSTPROCESSORWAREHOUSE_H
#define POSTPROCESSORWAREHOUSE_H

// MOOSE includes
#include "Warehouse.h"
#include "MooseTypes.h"
#include "Postprocessor.h"

// C++ includes
#include <vector>
#include <map>
#include <set>

// Forward declarations
class ElementPostprocessor;
class InternalSidePostprocessor;
class NodalPostprocessor;
class SidePostprocessor;
class GeneralPostprocessor;

/**
 * Holds postprocessors and provides some services
 */
class PostprocessorWarehouse : public Warehouse<Postprocessor>
{
public:
  PostprocessorWarehouse();
  virtual ~PostprocessorWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Get the list of all  elemental postprocessors
   * @param block_id Block ID
   * @return The list of all elemental postprocessors
   */
  const std::vector<ElementPostprocessor *> & elementPostprocessors(SubdomainID block_id) const;

  /**
   * Get the list of side postprocessors
   * @param boundary_id Boundary ID
   * @return The list of side postprocessors
   */
  const std::vector<SidePostprocessor *> & sidePostprocessors(BoundaryID boundary_id) const;

  /**
   * Get the list of nodal postprocessors
   * @param boundary_id Boundary ID
   * @return The list of all nodal postprocessors
   */
  const std::vector<NodalPostprocessor *> & nodalPostprocessors(BoundaryID boundary_id) const;

  /**
   * Get the list of nodal postprocessors restricted on the specified subdomain
   * @param block_id Subdomain ID
   * @return The list of all block nodal postprocessors
   */
  const std::vector<NodalPostprocessor *> & blockNodalPostprocessors(SubdomainID block_id) const;

  /**
   * Get the list general postprocessors
   * @return The list of general postprocessors
   */
  const std::vector<GeneralPostprocessor *> & genericPostprocessors() const { return _generic_postprocessors; }

  /**
   * Get a pointer to a postprocessor
   * @param name The name of the postprocessor to retrieve
   * @return A postprocessor for the given name
   */
  Postprocessor * getPostprocessor(std::string name);

  /**
   * Add a postprocessor
   * @param postprocessor Postprocessor being added
   */
  void addPostprocessor(MooseSharedPointer<Postprocessor> & postprocessor);

  /**
   * Get the list of blocks with postprocessors
   * @return The list of block IDs with postprocessors
   */
  const std::set<SubdomainID> & blocks() const { return _block_ids_with_postprocessors; }

  /**
   * Get the list of boundary IDs with postprocessors
   * @return The list of boundary IDs with postprocessors
   */
  const std::set<BoundaryID> & boundaryIds() const { return _boundary_ids_with_postprocessors; }

  /**
   * Get the list of nodeset IDs with postprocessors
   * @return The list of nodeset IDs with postprocessors
   */
  const std::set<BoundaryID> & nodesetIds() const { return _nodeset_ids_with_postprocessors; }

  /**
   * Get the list of subdomain IDs with *nodal* postprocessors
   * @return The list of subdomain IDs with postprocessors
   */
  const std::set<SubdomainID> & blockNodalIds() const { return _block_ids_with_nodal_postprocessors; }

protected:
  std::vector<ElementPostprocessor *> _all_element_postprocessors;
  std::vector<NodalPostprocessor *> _all_nodal_postprocessors;
  std::vector<SidePostprocessor *> _all_side_postprocessors;
  std::vector<InternalSidePostprocessor *> _all_internal_side_postprocessors;
  std::vector<GeneralPostprocessor *> _all_generic_postprocessors;

  std::map<SubdomainID, std::vector<ElementPostprocessor *> > _element_postprocessors;
  std::map<SubdomainID, std::vector<InternalSidePostprocessor *> > _internal_side_postprocessors;
  std::map<BoundaryID, std::vector<SidePostprocessor *> > _side_postprocessors;
  std::map<BoundaryID, std::vector<NodalPostprocessor *> > _nodal_postprocessors;
  // Block restricted nodal pps
  std::map<SubdomainID, std::vector<NodalPostprocessor *> > _block_nodal_postprocessors;

  std::vector<GeneralPostprocessor *> _generic_postprocessors;

  /// All of the block ids that have postprocessors specified to act on them
  std::set<SubdomainID> _block_ids_with_postprocessors;
  /// All of the boundary ids that have postprocessors specified to act on them
  std::set<BoundaryID> _boundary_ids_with_postprocessors;
  /// All of the nodeset ids that have postprocessors specified to act on them
  std::set<BoundaryID> _nodeset_ids_with_postprocessors;
  /// All of the block ids that have nodal postprocessors specified to act on them
  std::set<SubdomainID> _block_ids_with_nodal_postprocessors;

private:
  /// Hold shared pointers for automatic cleanup
  std::vector<MooseSharedPointer<Postprocessor> > _all_ptrs;
};

#endif // POSTPROCESSORWAREHOUSE_H
