/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINTRACKER_H
#define GRAINTRACKER_H

#include "FeatureFloodCount.h"
#include "GrainTrackerInterface.h"

// libMesh includes
#include "libmesh/mesh_tools.h"
#include "libmesh/auto_ptr.h"

class GrainTracker;
class EBSDReader;
struct GrainDistance;

template<>
InputParameters validParams<GrainTracker>();

class GrainTracker : public FeatureFloodCount, public GrainTrackerInterface
{
public:
  GrainTracker(const InputParameters & parameters);
  virtual ~GrainTracker();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  struct CacheValues
  {
    Real current;
    Real old;
    Real older;
  };

  enum REMAP_CACHE_MODE
  {
    FILL,
    USE,
    BYPASS
  };

  /**
   * Accessor for retrieving nodal field information (unique grains or variable indicies)
   * @param node_id the node identifier for which to retrieve field data
   * @param var_idx when using multi-map mode, the map number from which to retrieve data.
   * @param show_var_coloring pass true to view variable index for a region, false for unique grain information
   * @return the nodal value
   */
  virtual Real getEntityValue(dof_id_type node_id, FIELD_TYPE field_type, unsigned int var_idx=0) const override;

  /**
   * Returns a list of active unique grains for a particular elem based on the node numbering.  The outer vector
   * holds the ith node with the inner vector holds the list of active unique grains.
   * (unique_grain_id, variable_idx)
   */
  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const override;

protected:
  virtual void updateFieldInfo() override;

  /**
   * This method is called when a new grain is detected. It can be overridden by a derived class to handle
   * setting new properties on the newly created grain.
   */
  virtual void newGrainCreated(unsigned int new_grain_idx);

  /**
   * This method serves two purposes:
   * 1) When the tracking phase starts (_t_step == _tracking_step) it assigns a unique id to every FeatureData object
   *    found by the FeatureFloodCount object. If an EBSDReader is linked into the GrainTracker the information from the
   *    reader is used to assign grain information, otherwise it's ordered by each Feature's "minimum entity id" and
   *    assigned a non-negative integer.
   *
   * 2) On subsequent time_steps, incoming FeatureData objects are compared to previous time_step information to
   *    track grains between time steps.
   *
   * This method updates the _unique_grains datastructure.
   */
  void trackGrains();

  /**
   * This method is called after trackGrains to remap grains that are too close to each other.
   */
  void remapGrains();

  /**
   * Populates and sorts a min_distances vector with the minimum distances to all grains in the simulation
   * for a given grain. There are _vars.size() entries in the outer vector, one for each order parameter.
   * A list of grains with the same OP are ordered in lists per OP.
   */
  void computeMinDistancesFromGrain(FeatureData & grain, std::vector<std::list<GrainDistance> > & min_distances);

  /**
   * This is the recursive part of the remapping algorithm. It attempts to remap a grain to a new index and recurses until max_depth
   * is reached.
   */
  bool attemptGrainRenumber(FeatureData & grain, unsigned int grain_idx, unsigned int depth, unsigned int max);

  /**
   * A routine for moving all of the solution values from a given grain to a new variable number. It is called
   * with different modes to only cache, or actually do the work, or bypass the cache altogether.
   */
  void swapSolutionValues(FeatureData &  grain, unsigned int var_idx, std::map<Node *, CacheValues> & cache,
                          REMAP_CACHE_MODE cache_mode, unsigned int depth);

  /**
   * Helper method for actually performing the swaps.
   */
  void swapSolutionValuesHelper(Node * curr_node, unsigned int curr_var_idx, unsigned int new_var_idx, std::map<Node *, CacheValues> & cache,
                                REMAP_CACHE_MODE cache_mode);

  /**
   * This method returns the minimum periodic distance between two vectors of bounding boxes. If the bounding boxes overlap
   * the result is always -1.0.
   */
  Real boundingRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2) const;

  /**
   * This method returns the minimum periodic distance between the centroids of two vectors of bounding boxes.
   */
  Real centroidRegionDistance(std::vector<MeshTools::BoundingBox> & bboxes1, std::vector<MeshTools::BoundingBox> bboxes2) const;

  /**
   * This method colors neighbors of halo entries to expand the halo as desired for a given simulation.
   */
  void expandHalos();

  /**
   * Calculate the volume of each grain maintaining proper order and dumping results to CSV
   */
  virtual void calculateBubbleVolumes() override;

  /*************************************************
   *************** Data Structures *****************
   ************************************************/

  /// The timestep to begin tracking grains
  const int _tracking_step;

  /// The thickness of the halo surrounding each grain
  const unsigned int _halo_level;

  /// Depth of renumbing recursion (a depth of zero means no recursion)
  static const unsigned int _max_renumbering_recursion = 2;

  /// Inidicates whether remapping should be done or not (remapping is independent of tracking)
  const bool _remap;

  /// A reference to the nonlinear system (used for retrieving solution vectors)
  NonlinearSystem & _nl;

  /// This data structure holds the map of unique grains.  The information is updated each timestep to track grains over time.
  std::map<unsigned int, FeatureData> & _unique_grains;

  /**
   * This data structure holds unique grain to EBSD data map information. It's possible when using 2D scans of 3D microstructures
   * to end up with disjoint grains with the same orientation in a single slice. To properly handle this in the grain tracker
   * we need yet another map that takes a unique_grain number and retrieves the proper EBSD numbering (non-unique)
   */
  std::map<unsigned int, unsigned int> _unique_grain_to_ebsd_num;

  /// Optional ESBD Reader
  const EBSDReader * _ebsd_reader;

  bool _compute_op_maps;

  /**
   * Data structure for active order parameter information on elements:
   * elem_id -> a vector of pairs each containing the grain number and the variable index representing that grain
   */
  std::map<dof_id_type, std::vector<std::pair<unsigned int, unsigned int> > > _elemental_data;
};


/**
 * This struct is used to hold distance information to other grains in the simulation. It is used
 * for sorting and during the remapping algorithm.
 */
struct GrainDistance
{
  GrainDistance();
  GrainDistance(Real distance, unsigned int grain_id, unsigned int var_index);

  // Copy constructors
  GrainDistance(const GrainDistance & f) = default;
  GrainDistance & operator=(const GrainDistance & f) = default;

  // Move constructors
  GrainDistance(GrainDistance && f) = default;
  GrainDistance & operator=(GrainDistance && f) = default;

  bool operator<(const GrainDistance & rhs) const;

  Real _distance;
  unsigned int _grain_id;
  unsigned int _var_index;
};

#endif
