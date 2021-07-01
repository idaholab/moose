//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"

#include "libmesh/bounding_box.h"

class PolycrystalUserObjectBase;
struct GrainDistance;

class GrainTracker : public FeatureFloodCount, public GrainTrackerInterface
{
public:
  static InputParameters validParams();

  GrainTracker(const InputParameters & parameters);
  virtual ~GrainTracker();

  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  virtual std::size_t getTotalFeatureCount() const override;

  // Struct used to transfer minimal data to all ranks
  struct PartialFeatureData
  {
    BoundaryIntersection boundary_intersection;
    unsigned int id;
    Point centroid;
    Status status;
  };

  struct CacheValues
  {
    Real current;
    Real old;
    Real older;
  };

  enum class RemapCacheMode
  {
    FILL,
    USE,
    BYPASS
  };

  // GrainTrackerInterface methods
  virtual Real getEntityValue(dof_id_type node_id,
                              FieldType field_type,
                              std::size_t var_index = 0) const override;
  virtual const std::vector<unsigned int> &
  getVarToFeatureVector(dof_id_type elem_id) const override;
  virtual unsigned int getFeatureVar(unsigned int feature_id) const override;
  virtual std::size_t getNumberActiveGrains() const override;
  virtual Point getGrainCentroid(unsigned int grain_id) const override;
  virtual bool doesFeatureIntersectBoundary(unsigned int feature_id) const override;
  virtual bool doesFeatureIntersectSpecifiedBoundary(unsigned int feature_id) const override;
  virtual bool isFeaturePercolated(unsigned int feature_id) const override;
  virtual std::vector<unsigned int> getNewGrainIDs() const override;

protected:
  virtual void updateFieldInfo() override;
  virtual Real getThreshold(std::size_t current_index) const override;

  /**
   * This method extracts the necessary state from the passed in object necessary to continue
   * tracking grains. This method is meant to be used with the PolycrystalUserobjectBase class
   * that sets up initial conditions for Polycrystal simulations. We can use the state of that
   * object rather than rediscovering everything ourselves.
   */
  void prepopulateState(const FeatureFloodCount & ffc_object);

  /**
   */
  void communicateHaloMap();

  /**
   * When the tracking phase starts (_t_step == _tracking_step) it assigns a unique id to every
   * FeatureData object found by the FeatureFloodCount object.
   */
  void assignGrains();

  /**
   * On subsequent time_steps, incoming FeatureData objects are compared to previous time_step
   * information to track grains between time steps.
   *
   * This method updates the _feature_sets data structure.
   * This method should only be called on the root processor
   */
  void trackGrains();

  /**
   * This method is called when a new grain is detected. It can be overridden by a derived class to
   * handle setting new properties on the newly created grain.
   */
  virtual void newGrainCreated(unsigned int new_grain_id);

  /**
   * This method is called after trackGrains to remap grains that are too close to each other.
   */
  void remapGrains();

  /**
   * Broadcast essential Grain information to all processors. This method is used to get certain
   * attributes like centroids distributed and whether or not a grain intersects a boundary updated.
   */
  void broadcastAndUpdateGrainData();

  /**
   * Populates and sorts a min_distances vector with the minimum distances to all grains in the
   * simulation for a given grain. There are _vars.size() entries in the outer vector, one for
   * each order parameter. A list of grains with the same OP are ordered in lists per OP.
   */
  void computeMinDistancesFromGrain(FeatureData & grain,
                                    std::vector<std::list<GrainDistance>> & min_distances);

  /**
   * This is the recursive part of the remapping algorithm. It attempts to remap a grain to a new
   * index and recurses until max_depth is reached.
   */
  bool attemptGrainRenumber(FeatureData & grain, unsigned int depth, unsigned int max_depth);

  /**
   * A routine for moving all of the solution values from a given grain to a new variable number. It
   * is called with different modes to only cache, or actually do the work, or bypass the cache
   * altogether.
   */
  void swapSolutionValues(FeatureData & grain,
                          std::size_t new_var_index,
                          std::vector<std::map<Node *, CacheValues>> & cache,
                          RemapCacheMode cache_mode);

  /**
   * Helper method for actually performing the swaps.
   */
  void swapSolutionValuesHelper(Node * curr_node,
                                std::size_t curr_var_index,
                                std::size_t new_var_index,
                                std::vector<std::map<Node *, CacheValues>> & cache,
                                RemapCacheMode cache_mode);

  /**
   * This method returns the minimum periodic distance between two vectors of bounding boxes. If the
   * bounding boxes overlap the result is always -1.0.
   */
  Real boundingRegionDistance(std::vector<BoundingBox> & bboxes1,
                              std::vector<BoundingBox> & bboxes2) const;

  /**
   * This method returns the minimum periodic distance between the centroids of two vectors of
   * bounding boxes.
   */
  Real centroidRegionDistance(std::vector<BoundingBox> & bboxes1,
                              std::vector<BoundingBox> & bboxes2) const;

  /**
   * Retrieve the next unique grain number if a new grain is detected during trackGrains. This
   * method handles reserve order parameter indices properly. Direct access to the next index
   * should be avoided.
   */
  unsigned int getNextUniqueID();

  /*************************************************
   *************** Data Structures *****************
   ************************************************/

  /// The timestep to begin tracking grains
  const int _tracking_step;

  /// The thickness of the halo surrounding each grain
  const unsigned short _halo_level;

  /// Depth of renumbering recursion (a depth of zero means no recursion)
  const unsigned short _max_remap_recursion_depth;

  /// The number of reserved order parameters
  const unsigned short _n_reserve_ops;

  /// The cutoff index where if variable index >= this number, no remapping TO that variable
  /// will occur
  const std::size_t _reserve_op_index;

  /// The threshold above (or below) where a grain may be found on a reserve op field
  const Real _reserve_op_threshold;

  /// Inidicates whether remapping should be done or not (remapping is independent of tracking)
  const bool _remap;

  /// Indicates whether we should continue after a remap failure (will result in non-physical results)
  const bool _tolerate_failure;

  /// A reference to the nonlinear system (used for retrieving solution vectors)
  NonlinearSystemBase & _nl;

  /**
   * This data structure holds the map of unique grains from the previous time step.
   * The information is updated each timestep to track grains over time.
   */
  std::vector<FeatureData> _feature_sets_old;

  /// An optional IC UserObject which can provide initial data structures to this object.
  const PolycrystalUserObjectBase * const _poly_ic_uo;

  /**
   * Verbosity level controlling the amount of information printed to the console.
   */
  const short _verbosity_level;

  /**
   * Boolean to indicate the first time this object executes.
   * Note: _tracking_step isn't enough if people skip initial or execute more than once per step.
   */
  bool & _first_time;

  /**
   * Boolean to terminate with an error if a new grain is created during the simulation.
   * This is for simulations where new grains are not expected. Note, this does not impact
   * the initial callback to newGrainCreated() nor does it get triggered for splitting grains.
   */
  const bool _error_on_grain_creation;

private:
  /// Holds the first unique grain index when using _reserve_op (all the remaining indices are sequential)
  unsigned int _reserve_grain_first_index;

  /// The previous max grain id (needed to figure out which ids are new in a given step)
  unsigned int _old_max_grain_id;

  /// Holds the next "regular" grain ID (a grain found or remapped to the standard op vars)
  unsigned int & _max_curr_grain_id;

  /// Boolean to indicate whether this is a Steady or Transient solve
  const bool _is_transient;

  /// Data structure to hold element ID ranges when using Distributed Mesh (populated on rank 0 only)
  std::vector<std::pair<dof_id_type, dof_id_type>> _all_ranges;
};

/**
 * This struct is used to hold distance information to other grains in the simulation. It is used
 * for sorting and during the remapping algorithm.
 */
struct GrainDistance
{
  GrainDistance(Real distance, std::size_t var_index);

  GrainDistance(Real distance,
                std::size_t var_index,
                std::size_t grain_index,
                unsigned int grain_id);

  // Copy constructors
  GrainDistance(const GrainDistance & f) = default;
  GrainDistance & operator=(const GrainDistance & f) = default;

  // Move constructors
  GrainDistance(GrainDistance && f) = default;
  GrainDistance & operator=(GrainDistance && f) = default;

  bool operator<(const GrainDistance & rhs) const;

  Real _distance;
  std::size_t _var_index;
  std::size_t _grain_index;
  unsigned int _grain_id;
};

template <>
void dataStore(std::ostream & stream, GrainTracker::PartialFeatureData & feature, void * context);
template <>
void dataLoad(std::istream & stream, GrainTracker::PartialFeatureData & feature, void * context);
