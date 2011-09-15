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
  const std::vector<Postprocessor *> & elementPostprocessors(unsigned int block_id) { return _element_postprocessors[block_id]; }

  /**
   * Get the list of side postprocessors
   * @param boundary_id Boundary ID
   * @return The list of side postprocessors
   */
  const std::vector<Postprocessor *> & sidePostprocessors(unsigned int boundary_id) { return _side_postprocessors[boundary_id]; }

  /**
   * Get the list of nodal postprocessors
   * @param block_id Block ID
   * @return The list of all nodal postprocessors
   */
  const std::vector<Postprocessor *> & nodalPostprocessors() { return _nodal_postprocessors; }

  /**
   * Get the list general postprocessors
   * @return The list of general postprocessors
   */
  const std::vector<Postprocessor *> & genericPostprocessors() { return _generic_postprocessors; }

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
  const std::set<unsigned int> & blocks() { return _block_ids_with_postprocessors; }

  /**
   * Get the list of boundary IDs with postprocessors
   * @return The list of boundary IDs with postprocessors
   */
  const std::set<unsigned int> & boundaryIds() { return _boundary_ids_with_postprocessors; }

protected:
  std::map<unsigned int, std::vector<Postprocessor *> > _element_postprocessors;
  std::map<unsigned int, std::vector<Postprocessor *> > _side_postprocessors;
  std::vector<Postprocessor *> _nodal_postprocessors;
  
  std::vector<Postprocessor *> _generic_postprocessors;
  std::vector<Postprocessor *> _all_postprocessors;

  std::set<unsigned int> _block_ids_with_postprocessors;                ///< All of the block ids that have postprocessors specified to act on them
  std::set<unsigned int> _boundary_ids_with_postprocessors;             ///< All of the boundary ids that have postprocessors specified to act on them

};

#endif // POSTPROCESSORWAREHOUSE_H
