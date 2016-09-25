/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINTRACKERINTERFACE_H
#define GRAINTRACKERINTERFACE_H

#include "InputParameters.h"
#include "FeatureFloodCount.h"

// Forward declarations
class GrainTrackerInterface;

template<>
InputParameters validParams<GrainTrackerInterface>();

/**
 * This class is a fake grain tracker object, it will not actually track grains nor remap them
 * but will provide the same interface as the grain tracker and can be used as a lightweight
 * replacement when neither of those methods are needed. You may safely use this object anytime
 * you have at least as many order parameters as you do grains.
 */
class GrainTrackerInterface
{
public:
  /**
   * Accessor for retrieving either nodal or elemental information (unique grains or variable indicies)
   * @param entity_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain information
   * @return the entity value
   */
  virtual Real getEntityValue(dof_id_type entity_id, FeatureFloodCount::FieldType, std::size_t var_idx=0) const = 0;

  /**
   * Returns a list of active unique grains for a particular elem based on the node numbering.  The outer vector
   * holds the ith node with the inner vector holds the list of active unique grains.
   * (unique_grain_id, variable_idx)
   */
//  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const = 0;

  /**
   * Returns a list of active unique grains for a particular element. The vector is indexed by order parameter
   * with each entry containing an invalid uint (no grain active at that location) or a grain index
   * if it's active at that location.
   */
  virtual const std::vector<std::size_t> & getOpToGrainsVector(dof_id_type elem_id) const = 0;

  /**
   * Returns the number of active grains in a simulation. Note: This value will count
   * each piece of a split grain (often enountered in EBSD datasets).
   */
  virtual std::size_t getNumberActiveGrains() const = 0;

  /**
   * Returns the maximum grain index in use. This method can be used to size an array
   * or other data structure to maintain information about all grains (active and inactive)
   * in a simulation.
   */
  virtual std::size_t getTotalNumberGrains() const = 0;

  /**
   * Returns the actual grain ID given a grain index. Grain IDs are not required, and are often
   * not contiguous when using external data like EBSD data.
   */
//  virtual unsigned int getGrainID(std::size_t grain_index) const = 0;

  /**
   * Returns the centroid for the given grain number.
   */
  virtual Point getGrainCentroid(unsigned int grain_id) const = 0;
};

#endif
