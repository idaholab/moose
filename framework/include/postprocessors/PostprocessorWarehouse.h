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

#include "Postprocessor.h"

#include <vector>
#include <map>
#include <set>

class ElementPostprocessor;
class NodalPostprocessor;
class SidePostprocessor;
class GeneralPostprocessor;

/**
 * Holds postprocessors and provides some services
 */
class PostprocessorWarehouse
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
  const std::vector<ElementPostprocessor *> & elementPostprocessors(SubdomainID block_id) { return _element_postprocessors[block_id]; }

  /**
   * Get the list of side postprocessors
   * @param boundary_id Boundary ID
   * @return The list of side postprocessors
   */
  const std::vector<SidePostprocessor *> & sidePostprocessors(BoundaryID boundary_id) { return _side_postprocessors[boundary_id]; }

  /**
   * Get the list of nodal postprocessors
   * @param boundary_id Boundary ID
   * @return The list of all nodal postprocessors
   */
  const std::vector<NodalPostprocessor *> & nodalPostprocessors(BoundaryID boundary_id) { return _nodal_postprocessors[boundary_id]; }

  /**
   * Get the list general postprocessors
   * @return The list of general postprocessors
   */
  const std::vector<GeneralPostprocessor *> & genericPostprocessors() { return _generic_postprocessors; }

  /**
   * Get the list of all postprocessors
   * @return The list of all postprocessors
   */
  const std::vector<Postprocessor *> & all() { return _all_postprocessors; }

  /**
   * Add a postprocessor
   * @param postprocessor Postprocessor being added
   */
  void addPostprocessor(Postprocessor *postprocessor);

  /**
   * Get the list of blocks with postprocessors
   * @return The list of block IDs with postprocessors
   */
  const std::set<SubdomainID> & blocks() { return _block_ids_with_postprocessors; }

  /**
   * Get the list of boundary IDs with postprocessors
   * @return The list of boundary IDs with postprocessors
   */
  const std::set<BoundaryID> & boundaryIds() { return _boundary_ids_with_postprocessors; }

  /**
   * Get the list of nodeset IDs with postprocessors
   * @return The list of nodeset IDs with postprocessors
   */
  const std::set<BoundaryID> & nodesetIds() { return _nodeset_ids_with_postprocessors; }


protected:
  std::vector<ElementPostprocessor *> _all_element_postprocessors;
  std::vector<NodalPostprocessor *> _all_nodal_postprocessors;
  std::vector<SidePostprocessor *> _all_side_postprocessors;
  std::vector<GeneralPostprocessor *> _all_generic_postprocessors;

  std::map<SubdomainID, std::vector<ElementPostprocessor *> > _element_postprocessors;
  std::map<BoundaryID, std::vector<SidePostprocessor *> > _side_postprocessors;
  std::map<BoundaryID, std::vector<NodalPostprocessor *> > _nodal_postprocessors;

  std::vector<GeneralPostprocessor *> _generic_postprocessors;
  std::vector<Postprocessor *> _all_postprocessors;

  /// All of the block ids that have postprocessors specified to act on them
  std::set<SubdomainID> _block_ids_with_postprocessors;
  /// All of the boundary ids that have postprocessors specified to act on them
  std::set<BoundaryID> _boundary_ids_with_postprocessors;
  /// All of the nodeset ids that have postprocessors specified to act on them
  std::set<BoundaryID> _nodeset_ids_with_postprocessors;

};

#endif // POSTPROCESSORWAREHOUSE_H
