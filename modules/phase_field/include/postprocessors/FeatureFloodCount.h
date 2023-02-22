//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Coupleable.h"
#include "GeneralPostprocessor.h"
#include "InfixIterator.h"
#include "MooseVariableDependencyInterface.h"
#include "BoundaryRestrictable.h"

#include <iterator>
#include <list>
#include <set>
#include <vector>

#include "libmesh/bounding_box.h"
#include "libmesh/periodic_boundaries.h"

// External includes
#include "bitmask_operators.h"

// Forward Declarations
class MooseMesh;

/**
 * This object will mark nodes or elements of continuous regions all with a unique number for the
 * purpose of counting or "coloring" unique regions in a solution.  It is designed to work with
 * either a single variable, or multiple variables.
 *
 * Note:  When inspecting multiple variables, those variables must not have regions of interest
 *        that overlap or they will not be correctly colored.
 */
class FeatureFloodCount : public GeneralPostprocessor,
                          public Coupleable,
                          public MooseVariableDependencyInterface,
                          public BoundaryRestrictable
{
public:
  static InputParameters validParams();

  FeatureFloodCount(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void meshChanged() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;

  /// Return the number of active features
  std::size_t getNumberActiveFeatures() const;

  /// Returns the total feature count (active and inactive ids, useful for sizing vectors)
  virtual std::size_t getTotalFeatureCount() const;

  /// Returns a Boolean indicating whether this feature intersects _any_ boundary
  virtual bool doesFeatureIntersectBoundary(unsigned int feature_id) const;

  /// Returns a Boolean indicating whether this feature intersects boundaries in a user-supplied list
  virtual bool doesFeatureIntersectSpecifiedBoundary(unsigned int feature_id) const;

  /// Returns a Boolean indicating whether this feature is percolated (e.g. intersects at least two
  /// different boundaries from sets supplied by the user)
  virtual bool isFeaturePercolated(unsigned int feature_id) const;

  /// Returns the centroid of the designated feature (only supported without periodic boundaries)
  virtual Point featureCentroid(unsigned int feature_id) const;

  /**
   * Returns a list of active unique feature ids for a particular element. The vector is indexed by
   * variable number with each entry containing either an invalid size_t type (no feature active at
   * that location) or a feature id if the variable is non-zero at that location.
   */
  virtual const std::vector<unsigned int> & getVarToFeatureVector(dof_id_type elem_id) const;

  /// Returns the variable representing the passed in feature
  virtual unsigned int getFeatureVar(unsigned int feature_id) const;

  /// Returns the number of coupled varaibles
  std::size_t numCoupledVars() const { return _n_vars; }

  ///@{
  /// Constants used for invalid indices set to the max value of std::size_t type
  static const std::size_t invalid_size_t;
  static const unsigned int invalid_id;
  static const processor_id_type invalid_proc_id;
  ///@}

  /// Returns a const vector to the coupled variable pointers
  const std::vector<MooseVariable *> & getCoupledVars() const { return _vars; }

  /// Returns a const vector to the coupled MooseVariableFEBase pointers
  const std::vector<MooseVariableFEBase *> & getFECoupledVars() const { return _fe_vars; }

  enum class FieldType
  {
    UNIQUE_REGION,
    VARIABLE_COLORING,
    GHOSTED_ENTITIES,
    HALOS,
    CENTROID,
    ACTIVE_BOUNDS,
    INTERSECTS_SPECIFIED_BOUNDARY,
  };

  // Retrieve field information
  virtual Real
  getEntityValue(dof_id_type entity_id, FieldType field_type, std::size_t var_index = 0) const;

  inline bool isElemental() const { return _is_elemental; }

  /// This enumeration is used to indicate status of the grains in the _unique_grains data structure
  enum class Status : unsigned char
  {
    CLEAR = 0x0,
    MARKED = 0x1,
    DIRTY = 0x2,
    INACTIVE = 0x4
  };

  /// This enumeration is used to inidacate status of boundary intersections.
  enum class BoundaryIntersection : unsigned char
  {
    NONE = 0x0,
    ANY_BOUNDARY = 0x1,
    PRIMARY_PERCOLATION_BOUNDARY = 0x2,
    SECONDARY_PERCOLATION_BOUNDARY = 0x4,
    SPECIFIED_BOUNDARY = 0x8
  };

  class FeatureData
  {
  public:
    /**
     * The primary underlying container type used to hold the data in each FeatureData.
     * Supported types are std::set<dof_id_type> or std::vector<dof_id_type>.
     *
     * Note: Testing has shown that vector container _may_ be slightly faster, but I
     * believe much more data needs to be gathered to be sure. Perhaps more work could be performed
     * to keep sorted sets to eliminate any extra work we do by occasionally performing a linear
     * find or resorting the vector could help. For now, vector it is.
     */
    using container_type = std::vector<dof_id_type>;

    FeatureData() : FeatureData(std::numeric_limits<std::size_t>::max(), Status::INACTIVE) {}

    FeatureData(std::size_t var_index,
                unsigned int local_index,
                processor_id_type rank,
                Status status)
      : FeatureData(var_index, status)
    {
      _orig_ids = {std::make_pair(rank, local_index)};
    }

    FeatureData(std::size_t var_index,
                Status status,
                unsigned int id = invalid_id,
                std::vector<BoundingBox> bboxes = {BoundingBox()})
      : _var_index(var_index),
        _id(id),
        _bboxes(bboxes), // Assume at least one bounding box
        _min_entity_id(DofObject::invalid_id),
        _vol_count(0),
        _status(status),
        _boundary_intersection(BoundaryIntersection::NONE)
    {
    }

    ///@{
    // Default Move constructors
    FeatureData(FeatureData && /* f */) = default;
    FeatureData & operator=(FeatureData && /* f */) = default;
    ///@}

    ///@{
    /**
     * Update the minimum and maximum coordinates of a bounding box
     * given a Point, Elem or BBox parameter.
     */
    void updateBBoxExtremes(MeshBase & mesh);
    void updateBBoxExtremes(BoundingBox & bbox, const BoundingBox & rhs_bbox);
    ///@}

    /**
     * Determines if any of this FeatureData's bounding boxes overlap with
     * the other FeatureData's bounding boxes.
     */
    bool boundingBoxesIntersect(const FeatureData & rhs) const;

    /**
     * The routine called to see if two features are mergeable:
     *  - Features must be represented by the same variable (_var_index)
     *  - Features must either intersect on halos or
     *  - Features must intersect on a periodic BC
     *
     *  Optimization: We may use the bounding boxes as a coarse-level check before checking
     *  halo intersection.
     */
    bool mergeable(const FeatureData & rhs) const;

    /**
     * This routine indicates whether two features can be consolidated, that is, one feature is
     * reasonably expected to be part of another. This is different than mergable in that a portion
     * of the feature is expected to be completely identical. This happens in the distributed work
     * scenario when a feature that is partially owned by a processor is merged on a different
     * processor (where local entities are not sent or available). However, later that feature
     * ends back up on the original processor and just needs to be consolidated.
     */
    bool canConsolidate(const FeatureData & rhs) const;

    ///@{
    /**
     * Determine if one of this FeaturesData's member sets intersects
     * the other FeatureData's corresponding set.
     */
    bool halosIntersect(const FeatureData & rhs) const;
    bool periodicBoundariesIntersect(const FeatureData & rhs) const;
    bool ghostedIntersect(const FeatureData & rhs) const;
    ///@}

    /**
     * Located the overlapping bounding box between this Feature and the
     * other Feature and expands that overlapping box accordingly.
     */
    void mergeBBoxes(std::vector<BoundingBox> & bboxes, bool physical_intersection);

    /**
     * Merges another Feature Data into this one. This method leaves rhs
     * in an inconsistent state.
     */
    void merge(FeatureData && rhs);

    /**
     * Consolidates features, i.e. merges local entities but leaves everything else untouched.
     */
    void consolidate(FeatureData && rhs);

    // TODO: Doco
    void clear();

    /// Comparison operator for sorting individual FeatureDatas
    bool operator<(const FeatureData & rhs) const
    {
      if (_id != invalid_id)
      {
        mooseAssert(rhs._id != invalid_id, "Asymmetric setting of ids detected during sort");

        // Sort based on ids
        return _id < rhs._id;
      }
      else
        // Sort based on processor independent information (mesh and variable info)
        return _var_index < rhs._var_index ||
               (_var_index == rhs._var_index && _min_entity_id < rhs._min_entity_id);
    }

    /// stream output operator
    friend std::ostream & operator<<(std::ostream & out, const FeatureData & feature);

    /// Holds the ghosted ids for a feature (the ids which will be used for stitching
    container_type _ghosted_ids;

    /// Holds the local ids in the interior of a feature.
    /// This data structure is only maintained on the local processor
    container_type _local_ids;

    /// Holds the ids surrounding the feature
    container_type _halo_ids;

    /// Holds halo ids that extend onto a non-topologically connected surface
    container_type _disjoint_halo_ids;

    /// Holds the nodes that belong to the feature on a periodic boundary
    container_type _periodic_nodes;

    /// The Moose variable where this feature was found (often the "order parameter")
    std::size_t _var_index;

    /// An ID for this feature
    unsigned int _id;

    /// The vector of bounding boxes completely enclosing this feature
    /// (multiple used with periodic constraints)
    std::vector<BoundingBox> _bboxes;

    /// Original processor/local ids
    std::list<std::pair<processor_id_type, unsigned int>> _orig_ids;

    /// The minimum entity seen in the _local_ids, used for sorting features
    dof_id_type _min_entity_id;

    /// The count of entities contributing to the volume calculation
    std::size_t _vol_count;

    /// The centroid of the feature (average of coordinates from entities participating in
    /// the volume calculation)
    Point _centroid;

    /// The status of a feature (used mostly in derived classes like the GrainTracker)
    Status _status;

    /// Enumeration indicating boundary intersection status
    BoundaryIntersection _boundary_intersection;

    FeatureData duplicate() const { return FeatureData(*this); }

  private:
    ///@{
    /**
     * We do not expect these objects to ever be copied. This is important since they are stored in
     * standard containers directly. To enforce this, we are explicitly marking these methods
     * private. They can be triggered through an explicit call to "duplicate".
     */
    FeatureData(const FeatureData & /* f */) = default;
    FeatureData & operator=(const FeatureData & /* f */) = default;
    ///@}
  };

  /// Return a constant reference to the vector of all discovered features
  const std::vector<FeatureData> & getFeatures() const { return _feature_sets; }

protected:
  /**
   * Returns a Boolean indicating whether the entity is on one of the desired boundaries.
   */
  template <typename T>
  bool isBoundaryEntity(const T * entity) const;

  /**
   * This method is used to populate any of the data structures used for storing field data (nodal
   * or elemental). It is called at the end of finalize and can make use of any of the data
   * structures created during the execution of this postprocessor.
   */
  virtual void updateFieldInfo();

  /**
   * This method will check if the current entity is above the supplied threshold and "mark" it. It
   * will then inspect neighboring entities that are above the connecting threshold and add them to
   * the current feature.
   *
   * @return Boolean indicating whether a new feature was found while exploring the current entity.
   */
  bool flood(const DofObject * dof_object, std::size_t current_index);

  /**
   * Return the starting comparison threshold to use when inspecting an entity during the flood
   * stage.
   */
  virtual Real getThreshold(std::size_t current_index) const;

  /**
   * Return the "connecting" comparison threshold to use when inspecting an entity during the flood
   * stage.
   */
  virtual Real getConnectingThreshold(std::size_t current_index) const;

  /**
   * This method is used to determine whether the current entity value is part of a feature or not.
   * Comparisons can either be greater than or less than the threshold which is controlled via
   * input parameter.
   */
  bool compareValueWithThreshold(Real entity_value, Real threshold) const;

  /**
   * Method called during the recursive flood routine that should return whether or not the current
   * entity is part of the current feature (if one is being explored), or if it's the start
   * of a new feature.
   */
  virtual bool isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                             std::size_t & current_index,
                                             FeatureData *& feature,
                                             Status & status,
                                             unsigned int & new_id);

  /**
   * This method takes all of the partial features and expands the local, ghosted, and halo sets
   * around those regions to account for the diffuse interface. Rather than using any kind of
   * recursion here, we simply expand the region by all "point" neighbors from the actual grain
   * cells since all point neighbors will contain contributions to the region.
   */
  void expandPointHalos();

  /**
   * This method expands the existing halo set by some width determined by the passed in value.
   * This method does NOT mask off any local IDs.
   */
  void expandEdgeHalos(unsigned int num_layers_to_expand);

  ///@{
  /**
   * These two routines are utility routines used by the flood routine and by derived classes for
   * visiting neighbors. Since the logic is different for the elemental versus nodal case it's
   * easier to split them up.
   */
  void visitNodalNeighbors(const Node * node, FeatureData * feature, bool expand_halos_only);
  void visitElementalNeighbors(const Elem * elem,
                               FeatureData * feature,
                               bool expand_halos_only,
                               bool disjoint_only);
  ///@}

  /**
   * The actual logic for visiting neighbors is abstracted out here. This method is templated to
   * handle the Nodal
   * and Elemental cases together.
   */
  template <typename T>
  void visitNeighborsHelper(const T * curr_entity,
                            std::vector<const T *> neighbor_entities,
                            FeatureData * feature,
                            bool expand_halos_only,
                            bool topological_neighbor,
                            bool disjoint_only);

  /**
   * This routine uses the local flooded data to build up the local feature data structures
   * (_partial feature_sets). This routine does not perform any communication so the
   * _partial_feature_sets data structure will only contain information from the local processor
   * after calling this routine. Any existing data in the _partial_feature_sets structure is
   * destroyed by calling this routine.
   *
   * _partial_feature_sets layout:
   * The outer vector is sized to one when _single_map_mode == true, otherwise it is sized for the
   * number of coupled variables. The inner list represents the flooded regions (local only after
   * this call but fully populated after parallel communication and stitching).
   */
  virtual void prepareDataForTransfer();

  /**
   * This routines packs the _partial_feature_sets data into a structure suitable for parallel
   * communication operations.
   */
  void serialize(std::string & serialized_buffer, unsigned int var_num = invalid_id);

  /**
   * This routine takes the vector of byte buffers (one for each processor), deserializes them
   * into a series of FeatureSet objects, and appends them to the _feature_sets data structure.
   *
   * Note: It is assumed that local processor information may already be stored in the _feature_sets
   * data structure so it is not cleared before insertion.
   */
  void deserialize(std::vector<std::string> & serialized_buffers,
                   unsigned int var_num = invalid_id);

  /**
   * This routine is called on the primary rank only and stitches together the partial
   * feature pieces seen on any processor.
   */
  virtual void mergeSets();

  /**
   * This method consolidates all of the merged information from _partial_feature_sets into
   * the _feature_sets vectors.
   */
  virtual void
  consolidateMergedFeatures(std::vector<std::list<FeatureData>> * saved_data = nullptr);

  /**
   * Method for determining whether two features are mergeable. This routine exists because
   * derived classes may need to override this function rather than use the mergeable method
   * in the FeatureData object.
   */
  virtual bool areFeaturesMergeable(const FeatureData & f1, const FeatureData & f2) const;

  /**
   * Returns a number indicating the number of merge helpers when running in parallel based
   * on certain implementer decided criteria. This is a communication versus computation
   * trade-off that we are almost always willing to make except for small problems. The
   * decision however may be more complicated for some derived classes.
   */
  virtual processor_id_type numberOfDistributedMergeHelpers() const;

  /**
   * This routine handles all of the serialization, communication and deserialization of the data
   * structures containing FeatureData objects.
   */
  void communicateAndMerge();

  virtual void restoreOriginalDataStructures(std::vector<std::list<FeatureData>> &) {}

  /**
   * Sort and assign ids to features based on their position in the container after sorting.
   */
  void sortAndLabel();

  /**
   * Calls buildLocalToGlobalIndices to build the individual local to global indicies for each rank
   * and scatters that information to all ranks. Finally, the non-primary ranks update their own
   * data structures to reflect the global mappings.
   */
  void scatterAndUpdateRanks();

  /**
   * This routine populates a stacked vector of local to global indices per rank and the associated
   * count vector for scattering the vector to the ranks. The individual vectors can be different
   * sizes. The ith vector will be distributed to the ith processor including the primary rank.
   * e.g.
   * [ ... n_0 ] [ ... n_1 ] ... [ ... n_m ]
   *
   * It is intended to be overridden in derived classes.
   */
  virtual void buildLocalToGlobalIndices(std::vector<std::size_t> & local_to_global_all,
                                         std::vector<int> & counts) const;

  /**
   * This method builds a lookup map for retrieving the right local feature (by index) given a
   * global index or id. max_id is passed to size the vector properly and may or may not be a
   * globally consistent number. The assumption is that any id that is later queried from this
   * object that is higher simply doesn't exist on the local processor.
   */
  void buildFeatureIdToLocalIndices(unsigned int max_id);

  /**
   * Helper routine for clearing up data structures during initialize and prior to parallel
   * communication.
   */
  virtual void clearDataStructures();

  /**
   * Update the feature's attributes to indicate boundary intersections
   */
  void updateBoundaryIntersections(FeatureData & feature) const;

  /**
   * This routine adds the periodic node information to our data structure prior to packing the data
   * this makes those periodic neighbors appear much like ghosted nodes in a multiprocessor setting
   */
  void appendPeriodicNeighborNodes(FeatureData & feature) const;

  /**
   * This routine updates the _region_offsets variable which is useful for quickly determining
   * the proper global number for a feature when using multimap mode
   */
  void updateRegionOffsets();

  /*************************************************
   *************** Data Structures *****************
   ************************************************/
  /// The vector of coupled in variables
  std::vector<MooseVariableFEBase *> _fe_vars;
  /// The vector of coupled in variables cast to MooseVariable
  std::vector<MooseVariable *> _vars;

  /// Reference to the dof_map containing the coupled variables
  const DofMap & _dof_map;

  /// The threshold above (or below) where an entity may begin a new region (feature)
  const Real _threshold;
  Real _step_threshold;

  /// The threshold above (or below) which neighboring entities are flooded
  /// (where regions can be extended but not started)
  const Real _connecting_threshold;
  Real _step_connecting_threshold;

  /// A reference to the mesh
  MooseMesh & _mesh;

  /**
   * This variable is used to build the periodic node map.
   * Assumption: We are going to assume that either all variables are periodic or none are.
   *             This assumption can be relaxed at a later time if necessary.
   */
  unsigned long _var_number;

  /// This variable is used to indicate whether or not multiple maps are used during flooding
  const bool _single_map_mode;

  const bool _condense_map_info;

  /// This variable is used to indicate whether or not we identify features with
  /// unique numbers on multiple maps
  const bool _global_numbering;

  /// This variable is used to indicate whether the maps will contain unique region
  /// information or just the variable numbers owning those regions
  const bool _var_index_mode;

  /// Indicates whether or not to communicate halo map information with all ranks
  const bool _compute_halo_maps;

  /// Indicates whether or not the var to feature map is populated.
  const bool _compute_var_to_feature_map;

  /**
   * Use less-than when comparing values against the threshold value.
   * True by default.  If false, then greater-than comparison is used
   * instead.
   */
  const bool _use_less_than_threshold_comparison;

  // Convenience variable holding the number of variables coupled into this object
  const std::size_t _n_vars;

  /// Convenience variable holding the size of all the datastructures size by the number of maps
  const std::size_t _maps_size;

  /// Convenience variable holding the number of processors in this simulation
  const processor_id_type _n_procs;

  /**
   * This variable keeps track of which nodes have been visited during execution.  We don't use the
   * _feature_map for this since we don't want to explicitly store data for all the unmarked nodes
   * in a serialized datastructures.
   * This keeps our overhead down since this variable never needs to be communicated.
   */
  std::vector<std::set<dof_id_type>> _entities_visited;

  /**
   * This map keeps track of which variables own which nodes.  We need a vector of them for multimap
   * mode where multiple variables can own a single mode.
   *
   * Note: This map is only populated when "show_var_coloring" is set to true.
   */
  std::vector<std::map<dof_id_type, int>> _var_index_maps;

  /// The data structure used to find neighboring elements give a node ID
  std::unordered_map<dof_id_type, std::vector<const Elem *>> _nodes_to_elem_map;

  /// The number of features seen by this object per map
  std::vector<unsigned int> _feature_counts_per_map;

  /// The number of features seen by this object (same as summing _feature_counts_per_map)
  unsigned int _feature_count;

  /**
   * The data structure used to hold partial and communicated feature data, during the discovery and
   * merging phases. The outer vector is indexed by map number (often variable number). The inner
   * list is an unordered list of partially discovered features.
   */
  std::vector<std::list<FeatureData>> _partial_feature_sets;

  /**
   * The data structure used to hold the globally unique features. The sorting of the vector is
   * implementation defined and may not correspond to anything useful. The ID of each feature should
   * be queried from the FeatureData objects.
   */
  std::vector<FeatureData> & _feature_sets;

  /**
   * Derived objects (e.g. the GrainTracker) may require restartable data to track information
   * across time steps. The FeatureFloodCounter however does not. This container is here so that
   * we have the flexabilty to switch between volatile and non-volatile storage. The _feature_sets
   * data structure can conditionally refer to this structure or a MOOSE-provided structure, which
   * is backed up.
   */
  std::vector<FeatureData> _volatile_feature_sets;

  /**
   * The feature maps contain the raw flooded node information and eventually the unique grain
   * numbers.  We have a vector of them so we can create one per variable if that level of detail
   * is desired.
   */
  std::vector<std::map<dof_id_type, int>> _feature_maps;

  /// The vector recording the local to global feature indices
  std::vector<std::size_t> _local_to_global_feature_map;

  /// The vector recording the grain_id to local index (several indices will contain invalid_size_t)
  std::vector<std::size_t> _feature_id_to_local_index;

  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries * _pbs;

  std::unique_ptr<PointLocatorBase> _point_locator;

  /// Average value of the domain which can optionally be used to find features in a field
  const PostprocessorValue & _element_average_value;

  /// The map for holding reconstructed ghosted element information
  std::map<dof_id_type, int> _ghosted_entity_ids;

  /**
   * The data structure for looking up halos around features. The outer vector is for splitting out
   * the information per variable. The inner map holds the actual halo information
   */
  std::vector<std::map<dof_id_type, int>> _halo_ids;

  /**
   * The data structure which is a list of nodes that are constrained to other nodes
   * based on the imposed periodic boundary conditions.
   */
  std::multimap<dof_id_type, dof_id_type> _periodic_node_map;

  /// The set of entities on the boundary of the domain used for determining
  /// if features intersect any boundary
  std::unordered_set<dof_id_type> _all_boundary_entity_ids;

  std::map<dof_id_type, std::vector<unsigned int>> _entity_var_to_features;

  std::vector<unsigned int> _empty_var_to_features;

  std::vector<BoundaryID> _primary_perc_bnds;
  std::vector<BoundaryID> _secondary_perc_bnds;

  std::vector<BoundaryID> _specified_bnds;

  /// Determines if the flood counter is elements or not (nodes)
  const bool _is_elemental;

  /// Indicates that this object should only run on one or more boundaries
  bool _is_boundary_restricted;

  /// Boundary element range pointer
  ConstBndElemRange * _bnd_elem_range;

  /// Convenience variable for testing primary rank
  const bool _is_primary;

private:
  template <class T>
  static inline void sort(std::set<T> & /*container*/)
  {
    // Sets are already sorted, do nothing
  }

  template <class T>
  static inline void sort(std::vector<T> & container)
  {
    std::sort(container.begin(), container.end());
  }

  template <class T>
  static inline void reserve(std::set<T> & /*container*/, std::size_t /*size*/)
  {
    // Sets are trees, no reservations necessary
  }

  template <class T>
  static inline void reserve(std::vector<T> & container, std::size_t size)
  {
    container.reserve(size);
  }

  template <class T>
  static inline bool contains(std::set<T> & container, const T & item)
  {
    return container.find(item) != container.end();
  }

  template <class T>
  static inline bool contains(std::vector<T> & container, const T & item)
  {
    for (const auto & cont_item : container)
      if (item == cont_item)
        return true;
    return false;
  }

  /// The data structure for maintaining entities to flood during discovery
  std::deque<const DofObject *> _entity_queue;
};

template <>
void dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context);
template <>
void dataStore(std::ostream & stream, BoundingBox & bbox, void * context);

template <>
void dataLoad(std::istream & stream, FeatureFloodCount::FeatureData & feature, void * context);
template <>
void dataLoad(std::istream & stream, BoundingBox & bbox, void * context);

template <>
struct enable_bitmask_operators<FeatureFloodCount::Status>
{
  static const bool enable = true;
};

template <>
struct enable_bitmask_operators<FeatureFloodCount::BoundaryIntersection>
{
  static const bool enable = true;
};
