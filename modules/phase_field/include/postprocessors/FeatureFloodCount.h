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

  virtual void initialize();
  virtual void execute();
//  virtual void threadJoin(const UserObject & y);
  virtual void finalize();
  virtual Real getValue();

  enum FIELD_TYPE
  {
  UNIQUE_REGION,
  VARIABLE_COLORING,
  ACTIVE_BOUNDS,
  CENTROID,
  GHOSTED_ELEMS
  };
  
  // Retrieve field information
  virtual Real getNodalValue(dof_id_type node_id, unsigned int var_idx=0, bool show_var_coloring=false) const;
  virtual Real getElementalValue(dof_id_type element_id) const;

  virtual Real getEntityValue(dof_id_type entity_id, FIELD_TYPE field_type, unsigned int var_idx=0) const;

//  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getNodalValues(dof_id_type /*node_id*/) const;
  virtual const std::vector<std::pair<unsigned int, unsigned int> > & getElementalValues(dof_id_type elem_id) const;

  inline bool isElemental() const { return _is_elemental; }
  
  class FooBar
  {
  public:
    std::set<dof_id_type> _entity_ids;
    unsigned int _var_idx;
    Point _max;
    Point _min;
  };

  
  class BubbleData
  {
  public:
    BubbleData() :
        _var_idx(std::numeric_limits<unsigned int>::max()),
        _intersects_boundary(false),
        _min_feature_id(DofObject::invalid_id)
    {}
    
    BubbleData(std::set<dof_id_type> & entity_ids, unsigned int var_idx) :
        _entity_ids(entity_ids),
        _var_idx(var_idx),
        _intersects_boundary(false),
        _min_feature_id(DofObject::invalid_id)
    {}

    void updateBBoxMin(const Point & min);
    void updateBBoxMax(const Point & max);

    std::set<dof_id_type> _entity_ids;
    std::set<dof_id_type> _periodic_nodes;
    unsigned int _var_idx;
    bool _intersects_boundary;
    MeshTools::BoundingBox _bbox;
    dof_id_type _min_feature_id;
  };

protected:
  /**
   * This method is used to populate any of the data structures used for storing field data (nodal or elemental).
   * It is called at the end of finalize and can make use of any of the data structures created during
   * the execution of this postprocessor.
   */
  virtual void updateFieldInfo();

  /**
   * This method will "mark" all entities on neighboring elements that
   * are above the supplied threshold. If live_region == -1, that means the
   * region is inactive (unmarked areas)
   */
  void flood(const DofObject *dof_object, int current_idx, int live_region);

  /**
   * These routines packs/unpack the _bubble_map data into a structure suitable for parallel
   * communication operations. See the comments in these routines for the exact
   * data structure layout.
   */
  void pack(std::string & packed_data);
  void unpack(const std::string & packed_data);

  /**
   * This routine merges the data in _bubble_sets from separate threads/processes to resolve
   * any bubbles that were counted as unique by multiple processors.
   */
  void mergeSets(bool use_periodic_boundary_info);

  /**
   * This routine broadcasts a std::list<BubbleData> to other ranks. It includes both the
   * serialization and de-serialization routines.
   * @param list the list to broadcast
   * @param owner_id the rank initiating the broadcast
   * @param map_num the number in the _bubble_sets datastructure that will be replaced by the results of the broadcast
   */
  void communicateOneList(std::list<BubbleData> & list, unsigned int owner_id, unsigned int map_num);

  /**
   * This routine adds the periodic node information to our data structure prior to packing the data
   * this makes those periodic neighbors appear much like ghosted nodes in a multiprocessor setting
   */
  void appendPeriodicNeighborNodes(BubbleData & data) const;

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

  // Attempt to make a lower bound computation of memory consumed by this object
  virtual unsigned long calculateUsage() const;

  template<typename T>
  static unsigned long bytesHelper(T container);

  void formatBytesUsed() const;

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
  unsigned int _var_number;

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

  /// Convienence variable holding the size of all the datastructures size by the number of maps
  const unsigned int _maps_size;

  /**
   * This variable keeps track of which nodes have been visited during execution.  We don't use the _bubble_map
   * for this since we don't want to explicitly store data for all the unmarked nodes in a serialized datastructures.
   * This keeps our overhead down since this variable never needs to be communicated.
   */
  std::vector<std::map<dof_id_type, bool> > _entities_visited;

  /**
   * The bubble maps contain the raw flooded node information and eventually the unique grain numbers.  We have a vector
   * of them so we can create one per variable if that level of detail is desired.
   */
  std::vector<std::map<dof_id_type, int> > _bubble_maps;

  /**
   * This map keeps track of which variables own which nodes.  We need a vector of them for multimap mode where
   * multiple variables can own a single mode.  Note: This map is only populated when "show_var_coloring" is set
   * to true.
   */
  std::vector<std::map<dof_id_type, int> > _var_index_maps;
  
//  /// The data structure used to marshall the data between processes and/or threads
//  std::vector<unsigned int> _packed_data;

  /// The data structure used to find neighboring elements give a node ID
  std::vector< std::vector< const Elem * > > _nodes_to_elem_map;

  /// This data structure is used to keep track of which bubbles are owned by which variables (index).
  std::vector<unsigned int> _region_to_var_idx;

  /// This data structure holds the offset value for unique bubble ids (updated inside of finalize)
  std::vector<unsigned int> _region_offsets;

  /**
   * The data structure used to join partial bubbles between processes and/or threads.  We may have a list of BubbleData
   * per variable in multi-map mode
   */
  std::vector<std::list<BubbleData> > _bubble_sets;

  /// The scalar counters used during the marking stage of the flood algorithm. Up to one per variable
  std::vector<unsigned int> _region_counts;

  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries *_pbs;

  /// Average value of the domain which can optionally be used to find bubbles in a field
  const PostprocessorValue & _element_average_value;

  std::set<dof_id_type> _ghosted_entity_ids;

  /**
   * The data structure which is a list of nodes that are constrained to other nodes
   * based on the imposed periodic boundary conditions.
   */
  std::multimap<dof_id_type, dof_id_type> _periodic_node_map;

  /**
   * The filename and filehandle used if bubble volumes are being recorded to a file.
   * MooseSharedPointer is used so we don't have to worry about cleaning up after ourselves...
   */
  std::map<std::string, MooseSharedPointer<std::ofstream> > _file_handles;

  /**
   * The vector hold the volume of each flooded bubble.  Note: this vector is only populated
   * when requested by passing a file name to write this information to.
   */
  std::vector<Real> _all_bubble_volumes;

  // Memory Usage
  const bool _track_memory;
  unsigned long _bytes_used;

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
};

template<typename T>
unsigned long
FeatureFloodCount::bytesHelper(T container)
{
  typename T::value_type t;
  return sizeof(t) * container.size();
}


template <class T>
void
FeatureFloodCount::writeCSVFile(const std::string file_name, const std::vector<T> data)
{
  if (processor_id() == 0)
  {
    // typdef makes subsequent code easier to read...
    typedef std::map<std::string, MooseSharedPointer<std::ofstream> >::iterator iterator_t;

    // Try to find the filename
    iterator_t handle_it = _file_handles.find(file_name);

    // If the file_handle isn't found, create it
    if (handle_it == _file_handles.end())
    {
      MooseUtils::checkFileWriteable(file_name);

      // Store the new filename in the map
      std::pair<iterator_t, bool> result =
        _file_handles.insert(std::make_pair(file_name, MooseSharedPointer<std::ofstream>(new std::ofstream(file_name.c_str()))));

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


#endif //FEATUREFLOODCOUNT_H
