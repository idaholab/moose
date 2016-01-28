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
   * Accessor for retrieving nodal field information (unique grains or variable indicies)
   * @param node_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain information
   * @return the nodal value
   */
//  virtual Real getNodalValue(dof_id_type node_id, unsigned int var_idx=0, bool show_var_coloring=false) const = 0;

  /**
   * Accessor for retrieving either nodal or elemental information (unique grains or variable indicies)
   * @param entity_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain information
   * @return the entity value
   */
  virtual Real getEntityValue(dof_id_type entity_id, FeatureFloodCount::FIELD_TYPE, unsigned int var_idx=0) const = 0;

  /**
   * Accessor for retrieving elemental field data (grain centroids).
   * @param element_id the element identifier for which to retrieve field data
   * @return the elemental value
   */
  virtual Real getElementalValue(dof_id_type element_id) const = 0;

  /**
   * Returns a list of active unique grains for a particular elem based on the node numbering.  The outer vector
   * holds the ith node with the inner vector holds the list of active unique grains.
   * (unique_grain_id, variable_idx)
   */
  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const = 0;
};

#endif
