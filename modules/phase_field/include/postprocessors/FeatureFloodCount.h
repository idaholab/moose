/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FEATUREFLOODCOUNT_H
#define FEATUREFLOODCOUNT_H

#include "GeneralPostprocessor.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "ZeroInterface.h"
#include "InfixIterator.h"

#include <list>
#include <vector>
#include <set>
#include <iterator>

#include "libmesh/periodic_boundaries.h"
#include "libmesh/mesh_tools.h"

//Forward Declarations
class FeatureFloodCount;
class MooseMesh;
class MooseVariable;

template<>
InputParameters validParams<FeatureFloodCount>();

/**
 * This object will mark nodes or elements of continuous regions all with a unique number for the purpose of
 * counting or "coloring" unique regions in a solution.  It is designed to work with either a
 * single variable, or multiple variables.
 *
 * Note:  When inspecting multiple variables, those variables must not have regions of interest
 *        that overlap or they will not be correctly colored.
 */
class FeatureFloodCount :
  public GeneralPostprocessor,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public ZeroInterface
{
public:
  FeatureFloodCount(const InputParameters & parameters);
  ~FeatureFloodCount();

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;

  virtual void meshChanged() override;

  enum FIELD_TYPE
  {
    UNIQUE_REGION,
    VARIABLE_COLORING,
    GHOSTED_ENTITIES,
    HALOS,
    ACTIVE_BOUNDS,
    CENTROID
  };

  // Retrieve field information
  virtual Real getEntityValue(dof_id_type entity_id, FIELD_TYPE field_type, unsigned int var_idx=0) const;

  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const;

  inline bool isElemental() const { return _is_elemental; }

  /// This enumeration is used to indicate status of the grains in the _unique_grains data structure
  enum STATUS
  {
    NOT_MARKED,
    MARKED,
    INACTIVE
  };

  struct FeatureData
  {
    FeatureData(unsigned int var_idx = std::numeric_limits<unsigned int>::max()) :
        _var_idx(var_idx),
        _bboxes(1), // Assume at least one bounding box
        _min_entity_id(DofObject::invalid_id),
        _status(NOT_MARKED),
        _merged(false),
        _intersects_boundary(false)
    {
    }

    /**
     * We do not expect these objects to ever be copied. This is important
     * since they are stored in standard containers directly. To enforce
     * this, we are explicitly deleting the copy constructor, and copy
     * assignment operator.
     */
    FeatureData(const FeatureData & f) = delete;
    FeatureData & operator=(const FeatureData & f) = delete;

    // Move constructors
    FeatureData(FeatureData && f) = default;
    FeatureData & operator=(FeatureData && f) = default;

    void updateBBoxMin(MeshTools::BoundingBox & bbox, const Point & min);
    void updateBBoxMax(MeshTools::BoundingBox & bbox, const Point & max);

    void inflateBoundingBoxes(RealVectorValue inflation_amount)
    {
      for (unsigned int i = 0; i < _bboxes.size(); ++i)
      {
        _bboxes[i].max() += inflation_amount;
        _bboxes[i].min() -= inflation_amount;
      }
    }

    bool isStichable(const FeatureData & rhs) const;

    void expandBBox(const FeatureData & rhs);

    bool operator<(const FeatureData & rhs) const
    {
      return _min_entity_id < rhs._min_entity_id;
    }

    friend std::ostream & operator<< (std::ostream & out, const FeatureData & feature);

    std::set<dof_id_type> _ghosted_ids;
    std::set<dof_id_type> _local_ids;
    std::set<dof_id_type> _halo_ids;
    std::set<dof_id_type> _periodic_nodes;
    unsigned int _var_idx;
    std::vector<MeshTools::BoundingBox> _bboxes;
    dof_id_type _min_entity_id;
    STATUS _status;
    bool _merged;
    bool _intersects_boundary;
  };

protected:
  /**
   * This method is used to populate any of the data structures used for storing field data (nodal or elemental).
   * It is called at the end of finalize and can make use of any of the data structures created during
   * the execution of this postprocessor.
   */
  virtual void updateFieldInfo();

  void inflateBoundingBoxes(RealVectorValue inflation_amount);

  /**
   * This method will "mark" all entities on neighboring elements that
   * are above the supplied threshold. If feature is NULL, we are exploring
   * for a new region to mark, otherwise we are in the recursive calls
   * currently marking a region.
   */
  void flood(const DofObject * dof_object, unsigned long current_idx, FeatureData * feature);

  // TODO: doco
  void visitElementalNeighbors(const Elem * elem, unsigned long current_idx, FeatureData * feature, bool recurse);
  void visitNodalNeighbors(const Node * elem, unsigned long current_idx, FeatureData * feature, bool recurse);

  /**
   * This routine uses the local flooded data to build up the local feature data structures (_feature_sets).
   * This routine does not perform any communication so the _feature_sets data structure will only contain
   * information from the local processor after calling this routine. Any existing data in the _feature_sets
   * structure is destroyed by calling this routine.
   *
   *
   * _feature_sets layout:
   * The outer vector is sized to one when _single_map_mode == true, otherwise it is sized for the number
   * of coupled variables. The inner list represents the flooded regions (local only after this call
   * but fully populated after parallel communication and stitching).
   */
  void populateDataStructuresFromFloodData();

  /**
   * These routines packs/unpack the _feature_map data into a structure suitable for parallel
   * communication operations. See the comments in these routines for the exact
   * data structure layout.
   */
  void serialize(std::string & serialized_buffer);
  void deserialize(std::vector<std::string> & serialized_buffers);

  /**
   * This routine merges the data in _feature_sets from separate threads/processes to resolve
   * any bubbles that were counted as unique by multiple processors.
   */
  void mergeSets(bool use_periodic_boundary_info);

  void communicateAndMerge();

  /**
   * This routine adds the periodic node information to our data structure prior to packing the data
   * this makes those periodic neighbors appear much like ghosted nodes in a multiprocessor setting
   */
  void appendPeriodicNeighborNodes(FeatureData & data) const;

  /**
   * This routine updates the _region_offsets variable which is useful for quickly determining
   * the proper global number for a bubble when using multimap mode
   */
  void updateRegionOffsets();

  /**
   * This routine uses the bubble_sets data structure to calculate the volume of each stored bubble.
   */
  virtual void calculateBubbleVolumes();

  /**
   * This routine writes out data to a CSV file.  It is designed to be extended to derived classes
   * but is used to write out bubble volumes for this class.
   */
  template<class T>
  void writeCSVFile(const std::string file_name, const std::vector<T> data);

  /**
   * This method detects whether two sets intersect without building a result set.  It exits as soon as
   * any intersection is detected.
   */
  template<class InputIterator>
  inline bool setsIntersect(InputIterator first1, InputIterator last1, InputIterator first2, InputIterator last2) const
    {
      while (first1 != last1 && first2 != last2)
      {
        if (*first1 == *first2)
          return true;

        if (*first1 < *first2)
          ++first1;
        else if (*first1 > *first2)
          ++first2;
      }
      return false;
    }

  /*************************************************
   *************** Data Structures *****************
   ************************************************/

  /// The vector of coupled in variables
  std::vector<MooseVariable *> _vars;

  /// The threshold above where a node may begin a new region (bubble)
  const Real _threshold;
  Real _step_threshold;

  /// The threshold above which neighboring nodes are flooded (where regions can be extended but not started)
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

  /// This variable is used to indicate whether or not we identify bubbles with unique numbers on multiple maps
  const bool _global_numbering;

  /// This variable is used to inidicate whether the maps will continue unique region information or just the variable numbers owning those regions
  const bool _var_index_mode;

  /**
   * Use less-than when comparing values against the threshold value.
   * True by default.  If false, then greater-than comparison is used
   * instead.
   */
  const bool _use_less_than_threshold_comparison;

  // Convenience variable holding the number of variables coupled into this object
  const unsigned long _n_vars;

  /// Convenience variable holding the size of all the datastructures size by the number of maps
  const unsigned long _maps_size;

  /// Convenience variable holding the number of processors in this simulation
  const processor_id_type _n_procs;

  /**
   * This variable keeps track of which nodes have been visited during execution.  We don't use the _feature_map
   * for this since we don't want to explicitly store data for all the unmarked nodes in a serialized datastructures.
   * This keeps our overhead down since this variable never needs to be communicated.
   */
  std::vector<std::map<dof_id_type, bool> > _entities_visited;

  /**
   * This map keeps track of which variables own which nodes.  We need a vector of them for multimap mode where
   * multiple variables can own a single mode.  Note: This map is only populated when "show_var_coloring" is set
   * to true.
   */
  std::vector<std::map<dof_id_type, int> > _var_index_maps;

  /// The data structure used to find neighboring elements give a node ID
  std::vector< std::vector< const Elem * > > _nodes_to_elem_map;

  // The number of features seen by this object
  unsigned int _feature_count;

  /**
   * The data structure used to hold partial and communicated feature data.
   * The data structure mirrors that found in _feature_sets, but contains
   * one additional vector indexed by processor id
   */
  std::vector<std::vector<std::vector<FeatureData> > > _partial_feature_sets;

  /**
   * The data structure used to hold the globally unique features. The outer vector
   * is indexed by variable number, the inner vector is indexed by feature number
   */
  std::vector<std::vector<FeatureData> > _feature_sets;

  /**
   * The feature maps contain the raw flooded node information and eventually the unique grain numbers.  We have a vector
   * of them so we can create one per variable if that level of detail is desired.
   */
  std::vector<std::map<dof_id_type, int> > _feature_maps;


  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries *_pbs;

  /// Average value of the domain which can optionally be used to find bubbles in a field
  const PostprocessorValue & _element_average_value;

  // TODO: Doco
  std::map<dof_id_type, int> _ghosted_entity_ids;
  std::map<dof_id_type, int> _halo_ids;

  /**
   * The data structure which is a list of nodes that are constrained to other nodes
   * based on the imposed periodic boundary conditions.
   */
  std::multimap<dof_id_type, dof_id_type> _periodic_node_map;

  /**
   * The filename and filehandle used if bubble volumes are being recorded to a file.
   * std::unique_ptr is used so we don't have to worry about cleaning up after ourselves...
   */
  std::map<std::string, std::unique_ptr<std::ofstream> > _file_handles;

  /**
   * The vector hold the volume of each flooded bubble.  Note: this vector is only populated
   * when requested by passing a file name to write this information to.
   */
  std::vector<Real> _all_feature_volumes;

  // Dummy value for unimplemented method
  static const std::vector<std::pair<unsigned int, unsigned int> > _empty;

  /**
   * Vector of length _maps_size to keep track of the total
   * boundary-intersecting bubble volume scaled by the total domain
   * volume for each variable.
   */
  std::vector<Real> _total_volume_intersecting_boundary;

  /**
   * If true, the FeatureFloodCount object also computes the
   * (normalized) volume of bubbles which intersect the boundary and
   * reports this value in the CSV file (if available).  Defaults to
   * false.
   */
  bool _compute_boundary_intersecting_volume;

  /**
   * Determines if the flood counter is elements or not (nodes)
   */
  bool _is_elemental;

  /// used for testing if a neighbor element is semilocal
  std::set<const Elem *> _semilocal_elem_list;

  /// flag that indicates if an up to date list of semilocal (active) elements has been built
  bool _semilocal_elem_list_built;
};

template <class T>
void
FeatureFloodCount::writeCSVFile(const std::string file_name, const std::vector<T> data)
{
  if (processor_id() == 0)
  {
    // Try to find the filename
    auto handle_it = _file_handles.find(file_name);

    // If the file_handle isn't found, create it
    if (handle_it == _file_handles.end())
    {
      MooseUtils::checkFileWriteable(file_name);

      // Store the new filename in the map
      auto result = _file_handles.insert(std::make_pair(file_name, std::unique_ptr<std::ofstream>(new std::ofstream(file_name.c_str()))));

      // Be sure that the insert worked!
      mooseAssert(result.second, "Insertion into _file_handles map failed!");

      // Set handle_it to be an iterator to the new file.
      handle_it = result.first;
    }

    // Get reference to the stream, makes syntax below much simpler
    std::ofstream & the_stream = *(handle_it->second);

    // Set formatting flags on the stream - technically we only need to do this once, but whatever.
    the_stream << std::scientific << std::setprecision(6);

    mooseAssert(the_stream.is_open(), "File handle is not open");

    std::copy(data.begin(), data.end(), infix_ostream_iterator<T>(the_stream, ", "));
    the_stream << std::endl;
  }
}

template<> void dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context);
template<> void dataStore(std::ostream & stream, MeshTools::BoundingBox & bbox, void * context);

template<> void dataLoad(std::istream & stream, FeatureFloodCount::FeatureData & feature, void * context);
template<> void dataLoad(std::istream & stream, MeshTools::BoundingBox & bbox, void * context);


#endif //FEATUREFLOODCOUNT_H
