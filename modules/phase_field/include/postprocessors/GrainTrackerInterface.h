//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "FeatureFloodCount.h"

// Forward declarations

/**
 * This class defines the interface for the GrainTracking objects.
 */
class GrainTrackerInterface
{
public:
  static InputParameters validParams();

  /**
   * Accessor for retrieving either nodal or elemental information (unique grains or variable
   * indicies)
   * @param entity_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain
   * information
   * @return the entity value
   */
  virtual Real getEntityValue(dof_id_type entity_id,
                              FeatureFloodCount::FieldType,
                              std::size_t var_index = 0) const = 0;

  /**
   * Returns a list of active unique feature ids for a particular element. The vector is indexed by
   * variable number with each entry containing either an invalid size_t type (no feature active at
   * that location) or a feature id if the variable is non-zero at that location.
   */
  virtual const std::vector<unsigned int> & getVarToFeatureVector(dof_id_type elem_id) const = 0;

  /**
   * Return the variable index (typically order parameter) for the given feature. Returns
   * "invalid_id" if the specified feature is inactive.
   */
  virtual unsigned int getFeatureVar(unsigned int feature_id) const = 0;

  /**
   * Returns the number of active grains current stored in the GrainTracker. This value is the same
   * value reported when the GrainTracker (FeatureFloodObject) is used as a Postprocessor.
   *
   * Note: This value will count each piece of a split grain (often encountered in EBSD data sets).
   */
  virtual std::size_t getNumberActiveGrains() const = 0;

  /**
   * Returns a number large enough to contain the largest ID for all grains in use. This method can
   * be used to size a vector or other data structure to maintain information about all grains
   * (active and inactive) in a simulation.
   */
  virtual std::size_t getTotalFeatureCount() const = 0;

  /**
   * Returns the centroid for the given grain number.
   */
  virtual Point getGrainCentroid(unsigned int grain_id) const = 0;

  /**
   * Returns a Boolean indicating whether this grain is in contact with any boundary of the domain
   */
  virtual bool doesFeatureIntersectBoundary(unsigned int grain_id) const = 0;

  /**
   * This method returns all of the new ids generated in an invocation of the GrainTracker.
   */
  virtual std::vector<unsigned int> getNewGrainIDs() const;
};
