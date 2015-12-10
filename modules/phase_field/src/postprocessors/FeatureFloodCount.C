/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FeatureFloodCount.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseUtils.h"

#include "NonlinearSystem.h"
#include "FEProblem.h"

//libMesh includes
#include "libmesh/dof_map.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point_locator_base.h"

#include <algorithm>
#include <limits>

#include <unistd.h> // sleep

template <typename T>
struct string_vector_inserter
  : std::iterator<std::output_iterator_tag, T>
{
  explicit string_vector_inserter(std::vector<T *> & vector) : _vector(vector)
    {
    }

  template <typename T2>
  void operator=(const T2 & value)
    {
      _vector.push_back(value);
    }

  string_vector_inserter& operator++() {
    return *this;
  }

  string_vector_inserter operator++(int) {
    return string_vector_inserter(*this);
  }

  // We don't return a reference-to-T here because we don't want to
  // construct one or have any of its methods called.
  string_vector_inserter& operator*() { return *this; }

private:
  std::vector<T *> & _vector;
};


/**
 * The following methods are specializations for using the libMesh::Parallel::packed_range_* routines
 * for std::strings. These are here because the dataLoad/dataStore routines create raw string
 * buffers that can be communicated in a standard way using packed ranges.
 */
namespace libMesh {
namespace Parallel {

/// BufferType<> specializations to return a buffer datatype to handle communication of std::strings
template <>
struct BufferType<const std::string *> {
  typedef char type;
};

///@{
/// packed_size to return the size of the packed (serialized) string
template<>
unsigned int packed_size(const std::string *, std::vector<char>::const_iterator in)
{
  // std::string is encoded as a 32-bit length followed by the content (char)
  return (static_cast<unsigned char>(in[0]) << 24) | (static_cast<unsigned char>(in[1]) << 16) | (static_cast<unsigned char>(in[2]) << 8) | (static_cast<unsigned char>(in[3]) << 0);

}

template<>
unsigned int packed_size(const std::string * s, std::vector<char>::iterator in)
{
  return packed_size(s, std::vector<char>::const_iterator(in));
}
///@}

/// packable size is called prior to serializing the string
template<>
unsigned int packable_size(const std::string * s, const void *)
{
  // String is encoded as a 32-bit length followed by the content (char)
  return s->size() + sizeof(uint32_t);
}

/// pack a single variable sized string onto the data buffer
template <>
void pack (const std::string * b, std::vector<char> & data, const void *)
{
  uint32_t size = sizeof(uint32_t) + b->size();

  data.push_back(size >> 24);
  data.push_back(size >> 16);
  data.push_back(size >> 8);
  data.push_back(size);

  // Copy the content to the buffer
  std::copy(b->begin(), b->end(), std::back_inserter(data));
}

/// unpack a single variable sized string from the data buffer
template <>
void unpack(std::vector<char>::const_iterator in, std::string ** out, void *)
{
//  uint32_t size = ((unsigned char)in[0] << 24) | (unsigned char)(in[1] << 16) | (unsigned char)(in[2] << 8) | (unsigned char)(in[3] << 0);
  uint32_t size = (static_cast<unsigned char>(in[0]) << 24) | (static_cast<unsigned char>(in[1]) << 16) | (static_cast<unsigned char>(in[2]) << 8) | (static_cast<unsigned char>(in[3]) << 0);

  /**
   * Start at the right place in the string (after the length parameter) and adjust the
   * size of the content by the same amount.
   */
  std::string * out_string = new std::string(&in[sizeof(uint32_t)], size - sizeof(uint32_t));

  // Advance the position in the buffer
  in += size;

  (*out) = out_string;
}

}
}


template<> void dataStore(std::ostream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  storeHelper(stream, feature._ghosted_ids, context);
  storeHelper(stream, feature._var_idx, context);
  storeHelper(stream, feature._bbox.min(), context);
  storeHelper(stream, feature._bbox.max(), context);
  storeHelper(stream, feature._min_feature_id, context);
}

template<> void dataLoad(std::istream & stream, FeatureFloodCount::FeatureData & feature, void * context)
{
  loadHelper(stream, feature._ghosted_ids, context);
  loadHelper(stream, feature._var_idx, context);
  loadHelper(stream, feature._bbox.min(), context);
  loadHelper(stream, feature._bbox.max(), context);
  loadHelper(stream, feature._min_feature_id, context);
}

template<>
InputParameters validParams<FeatureFloodCount>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredCoupledVar("variable", "The variable(s) for which to find connected regions of interests, i.e. \"bubbles\".");
  params.addParam<Real>("threshold", 0.5, "The threshold value for which a new bubble may be started");
  params.addParam<Real>("connecting_threshold", "The threshold for which an existing bubble may be extended (defaults to \"threshold\")");
  params.addParam<PostprocessorName>("elem_avg_value", "If supplied, will be used to find the scaled threshold of the bubble edges");
  params.addParam<bool>("use_single_map", true, "Determine whether information is tracked per coupled variable or consolidated into one (default: true)");
  params.addParam<bool>("condense_map_info", false, "Determines whether we condense all the node values when in multimap mode (default: false)");
  params.addParam<bool>("use_global_numbering", false, "Determine whether or not global numbers are used to label bubbles on multiple maps (default: false)");
  params.addParam<bool>("enable_var_coloring", false, "Instruct the UO to populate the variable index map.");
  params.addParam<bool>("use_less_than_threshold_comparison", true, "Controls whether bubbles are defined to be less than or greater than the threshold value.");
  params.addParam<FileName>("bubble_volume_file", "An optional file name where bubble volumes can be output.");
  params.addDeprecatedParam<bool>("track_memory_usage", false, "Track memory usage", "This parameter is no longer valid, please remove.");
  params.addParam<bool>("compute_boundary_intersecting_volume", false, "If true, also compute the (normalized) volume of bubbles which intersect the boundary");

  MooseEnum flood_type("NODAL ELEMENTAL", "NODAL");
  params.addParam<MooseEnum>("flood_entity_type", flood_type, "Determines whether the flood algorithm runs on nodes or elements");
  return params;
}

FeatureFloodCount::FeatureFloodCount(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    Coupleable(parameters, false),
    MooseVariableDependencyInterface(),
    ZeroInterface(parameters),
    _vars(getCoupledMooseVars()),
    _threshold(getParam<Real>("threshold")),
    _connecting_threshold(isParamValid("connecting_threshold") ? getParam<Real>("connecting_threshold") : getParam<Real>("threshold")),
    _mesh(_subproblem.mesh()),
    _var_number(_vars[0]->number()),
    _single_map_mode(getParam<bool>("use_single_map")),
    _condense_map_info(getParam<bool>("condense_map_info")),
    _global_numbering(getParam<bool>("use_global_numbering")),
    _var_index_mode(getParam<bool>("enable_var_coloring")),
    _use_less_than_threshold_comparison(getParam<bool>("use_less_than_threshold_comparison")),
    _maps_size(_single_map_mode ? 1 : _vars.size()),
    _pbs(NULL),
    _element_average_value(parameters.isParamValid("elem_avg_value") ? getPostprocessorValue("elem_avg_value") : _real_zero),
    _compute_boundary_intersecting_volume(getParam<bool>("compute_boundary_intersecting_volume")),
    _is_elemental(getParam<MooseEnum>("flood_entity_type") == "ELEMENTAL" ? true : false)
{
  _partial_feature_sets.resize(_app.n_processors());

//  // Size the data structures to hold the correct number of maps
//  for (unsigned int rank = 0; rank < _app.n_processors(); ++rank)
//    _partial_feature_sets[rank].resize(_maps_size);
  _feature_sets.resize(_maps_size);
  _feature_maps.resize(_maps_size);
  _region_counts.resize(_maps_size);
  _region_offsets.resize(_maps_size);

  if (_var_index_mode)
    _var_index_maps.resize(_maps_size);

  // This map is always size to the number of variables
  _entities_visited.resize(_vars.size());
}

FeatureFloodCount::~FeatureFloodCount()
{
}

void
FeatureFloodCount::initialize()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  // TODO: Can we do this in the constructor (i.e. are all objects necessary for this call in existance during ctor?)
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // Clear the bubble marking maps and region counters and other data structures
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    _feature_maps[map_num].clear();
    _feature_sets[map_num].clear();
    _region_counts[map_num] = 0;
    _entities_visited[map_num].clear();

    if (_var_index_mode)
      _var_index_maps[map_num].clear();
  }

  // TODO: use iterator
  for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
    _entities_visited[var_num].clear();

  // Reset the ownership structure
  _region_to_var_idx.clear();

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  // TODO: We might only need to build this once if adaptivity is turned off
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _var_number, _pbs);

  // Calculate the thresholds for this iteration
  _step_threshold = _element_average_value + _threshold;
  _step_connecting_threshold = _element_average_value + _connecting_threshold;

  _all_feature_volumes.clear();

  _ghosted_entity_ids.clear();

  // Populate the global ghosted entity data structure
  const MeshBase::element_iterator end = _mesh.getMesh().ghost_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().ghost_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;

    if (_is_elemental)
      _ghosted_entity_ids.insert(current_elem->id());
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
        _ghosted_entity_ids.insert(current_elem->get_node(i)->id());
    }
  }
  _communicator.set_union(_ghosted_entity_ids);
}

void
FeatureFloodCount::execute()
{
  const MeshBase::element_iterator end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != end; ++el)
  {
    const Elem * current_elem = *el;

    // Loop over elements or nodes
    if (_is_elemental)
    {
      for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
        flood(current_elem, var_num, -1 /* Designates unmarked region */);
    }
    else
    {
      unsigned int n_nodes = current_elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * current_node = current_elem->get_node(i);

        for (unsigned int var_num = 0; var_num < _vars.size(); ++var_num)
          flood(current_node, var_num, -1 /* Designates unmarked region */);
      }
    }
  }
}

void
FeatureFloodCount::finalize()
{
  // First we need to transform the raw data into a usable data structure
  populateDataStructuresFromFloodData();

  /*********************************************************************************
   *********************************************************************************
   * Begin Parallel Communication Section
   *********************************************************************************
   *********************************************************************************/

  // The processor local byte buffer
  std::string serialized_buffer;

  /**
   * The libMesh packed range routines handle the communication of the individual
   * string buffers. Here we need to create a container to hold our type
   * to serialize. It'll always be size one because we are sending a single
   * byte stream of all the data to other processors. The stream need not be
   * the same size on all processors.
   */
  std::vector<std::string *> send_buffers(1, &serialized_buffer);

  /**
   * Additionally we need to create a different container to hold the received
   * byte buffers. The container type need not match the send container type.
   * However, We do know the number of incoming buffers (num processors) so we'll
   * go ahead and use a vector.
   */
  std::vector<std::string *> recv_buffers;
  recv_buffers.reserve(_app.n_processors());

  serialize(&serialized_buffer);

  /**
   * Each processor needs information from all other processors to create a complete
   * global feature map.
   */
  _communicator.allgather_packed_range((void *)(NULL), send_buffers.begin(), send_buffers.end(),
                                       string_vector_inserter<std::string>(recv_buffers));

  deserialize(recv_buffers);

  /*********************************************************************************
   *********************************************************************************
   * End Parallel Communication Section
   *********************************************************************************
   *********************************************************************************/

  inflateBoundingBoxes();

  mergeSets(true);


  // Debugging
  unsigned int counter = 0;
  std::cout << "\n*********************************************************************************\nDATA ON PROCESSOR " << processor_id() << '\n';
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (std::vector<FeatureData>::iterator l_it = _feature_sets[map_num].begin(); l_it != _feature_sets[map_num].end(); ++l_it)
    {
      std::cout << "\nItem: " << ++counter << "\nGhosted Entities: ";
      for (std::set<dof_id_type>::const_iterator it = l_it->_ghosted_ids.begin();
           it != l_it->_ghosted_ids.end(); ++it)
        std::cout << *it << " ";
      std::cout << "\nInterior Entities: ";
      for (std::set<dof_id_type>::const_iterator it = l_it->_interior_ids.begin();
           it != l_it->_interior_ids.end(); ++it)
        std::cout << *it << " ";

      std::cout << "\nVar_idx: " << l_it->_var_idx;
      std::cout << "\nMax: " << l_it->_bbox.max();
      std::cout << "\nMin: " << l_it->_bbox.min();
      std::cout << "\nMin Entitity ID: " << l_it->_min_feature_id;
      std::cout << "\n\n";
    }

  exit(0);

  // Populate _feature_maps and _var_index_maps
  updateFieldInfo();

  // Update the region offsets so we can get unique bubble numbers in multimap mode
  updateRegionOffsets();

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data;
    data.reserve(_all_feature_volumes.size() + _total_volume_intersecting_boundary.size() + 2);

    // Insert the current timestep and the simulation time into the data vector
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());

    // Insert the (sorted) bubble volumes into the data vector
    data.insert(data.end(), _all_feature_volumes.begin(), _all_feature_volumes.end());

    // If we are computing the boundary-intersecting volumes, insert
    // those numbers into the normalized boundary-intersecting bubble
    // volumes into the data vector.
    if (_compute_boundary_intersecting_volume)
      data.insert(data.end(), _total_volume_intersecting_boundary.begin(), _total_volume_intersecting_boundary.end());

    // Finally, write the file
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }
}

Real
FeatureFloodCount::getValue()
{
  unsigned int count = 0;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    count += _feature_sets[map_num].size();

  return count;
}


Real
FeatureFloodCount::getNodalValue(dof_id_type node_id, unsigned int var_idx, bool show_var_coloring) const
{
  mooseDoOnce(mooseWarning("Please call getEntityValue instead"));

  return 0;
}

Real
FeatureFloodCount::getElementalValue(dof_id_type /*element_id*/) const
{
  mooseDoOnce(mooseWarning("Method not implemented"));

  return 0;
}

Real
FeatureFloodCount::getEntityValue(dof_id_type entity_id, FIELD_TYPE field_type, unsigned int var_idx) const
{
  mooseAssert(var_idx < _maps_size, "Index out of range");
//  mooseAssert(!show_var_coloring || _var_index_mode, "Cannot use \"show_var_coloring\" without \"enable_var_coloring\"");

  switch (field_type)
  {
  case UNIQUE_REGION:
  {
    std::map<dof_id_type, int>::const_iterator entity_it = _feature_maps[var_idx].find(entity_id);

    if (entity_it != _feature_maps[var_idx].end())
      return entity_it->second + _region_offsets[var_idx];
    else
      return -1;
  }

  case VARIABLE_COLORING:
  {
    std::map<dof_id_type, int>::const_iterator entity_it = _var_index_maps[var_idx].find(entity_id);

    if (entity_it != _var_index_maps[var_idx].end())
      return entity_it->second;
    else
      return -1;
  }

  case GHOSTED_ELEMS:
    return _ghosted_entity_ids.find(entity_id) != _ghosted_entity_ids.end();

  default:
    return 0;
  };
}

const std::vector<std::pair<unsigned int, unsigned int> > &
FeatureFloodCount::getElementalValues(dof_id_type /*elem_id*/) const
{
  mooseDoOnce(mooseWarning("Method not implemented"));
  return _empty;
}

void
FeatureFloodCount::populateDataStructuresFromFloodData()
{
  MeshBase & mesh = _mesh.getMesh();
  processor_id_type rank = processor_id();

  _partial_feature_sets[rank].clear();
  _partial_feature_sets[rank].resize(_maps_size);

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    // Destroy any existing data
    _partial_feature_sets[rank][map_num].clear();

    // Resize the inner vector for the number of local flooded regions
    _partial_feature_sets[rank][map_num].resize(_region_counts[map_num]);

    /**
     * Populate the feature data structure members.
     */
    for (unsigned int feature_num = 0; feature_num < _region_counts[map_num]; ++feature_num)
      _partial_feature_sets[rank][map_num][feature_num]._var_idx = _region_to_var_idx[feature_num];

    /**
     * Transform the flooded regions into feature sets
     *
     * map of entity_id to feature_num  -->  map_num [ feature_num [ set of entity_ids ] ]
     */
    std::map<dof_id_type, int>::const_iterator entity_end = _feature_maps[map_num].end();
    for (std::map<dof_id_type, int>::const_iterator entity_it = _feature_maps[map_num].begin(); entity_it != entity_end; ++entity_it)
    {
      // Local variables for clarity
      dof_id_type entity_id = entity_it->first;
      int feature_num = entity_it->second;

      /**
       * Processors only need enough information to stitch together the regions.
       * We'll keep ids on ghosted regions (overlap areas) separate from the "interior"
       * local ids.
       */
      if (_ghosted_entity_ids.find(entity_id) != _ghosted_entity_ids.end())
        _partial_feature_sets[rank][map_num][feature_num]._ghosted_ids.insert(entity_id);
      else
        _partial_feature_sets[rank][map_num][feature_num]._interior_ids.insert(entity_id);


      // Now retrieve the location of this entity for determining the bounding box region
      const Point & entity_point = _is_elemental ? mesh.elem(entity_id)->centroid() : mesh.node(entity_id);

      _partial_feature_sets[rank][map_num][feature_num].updateBBoxMin(entity_point);
      _partial_feature_sets[rank][map_num][feature_num].updateBBoxMax(entity_point);

      // Finally save off the min entity id present in the feature to uniquely identify the feature regardless of n_procs
      _partial_feature_sets[rank][map_num][feature_num]._min_feature_id = std::min(_partial_feature_sets[rank][map_num][feature_num]._min_feature_id, entity_id);
    }
  }
}

void
FeatureFloodCount::serialize(std::string * serialized_buffer)
{
//  if (processor_id() == 1)
//  {
//    unsigned int counter = 0;
//    std::cerr << "\n\nReady to Serialize Local Data:\n";
//    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//      for (std::vector<FeatureData>::iterator l_it = _feature_sets[map_num].begin(); l_it != _feature_sets[map_num].end(); ++l_it)
//      {
//        std::cerr << "Proc: " << processor_id() << "\nItem: " << ++counter << "\nGhosted Entities: ";
//        for (std::set<dof_id_type>::const_iterator it = l_it->_ghosted_ids.begin();
//             it != l_it->_ghosted_ids.end(); ++it)
//          std::cerr << *it << " ";
//        std::cerr << "\nInterior Entities: ";
//        for (std::set<dof_id_type>::const_iterator it = l_it->_interior_ids.begin();
//           it != l_it->_interior_ids.end(); ++it)
//          std::cerr << *it << " ";
//
//
//        std::cerr << "\nVar_idx: " << l_it->_var_idx;
//        std::cerr << "\nMax: " << l_it->_bbox.max();
//        std::cerr << "\nMin: " << l_it->_bbox.min();
//        std::cerr << "\nMin Entitity ID: " << l_it->_min_feature_id;
//        std::cerr << "\n\n";
//      }
//  }


  // stream for serializing the _partial_feature_sets data structure to a byte stream
  std::ostringstream oss;

  /**
   * Call the MOOSE serialization routines to serialize this processor's data.
   * Note: The _partial_feature_sets data structure will be empty for all other processors
   */
  dataStore(oss, _partial_feature_sets[processor_id()], this);

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer->assign(oss.str());
}

/**
 * This routine takes the vector of byte buffers (one for each processor), deserializes them
 * into a series of FeatureSet objects, and appends them to the _feature_sets data structure.
 *
 * Note: It is assumed that local processor information may already be stored in the _feature_sets
 * data structure so it is not cleared before insertion.
 */
void
FeatureFloodCount::deserialize(std::vector<std::string *> & serialized_buffers)
{
  // The input string stream used for deserialization
  std::istringstream iss;

  mooseAssert(serialized_buffers.size() == _app.n_processors(), "Unexpected size of serialized_buffers: " << serialized_buffers.size());

  for (unsigned int rank = 0; rank < serialized_buffers.size(); ++rank)
  {
    /**
     * We should already have the local processor data in the features data structure.
     * Don't unpack the local buffer again.
     */
    if (rank == processor_id())
      continue;

    iss.str(*serialized_buffers[rank]);   // populate the stream with a new buffer
    iss.clear();                          // reset the string stream state

    // Load the communicated data into all of the other processors' slots
    dataLoad(iss, _partial_feature_sets[rank], this);
  }

//  // Debugging
//  unsigned int counter = 0;
//  std::cout << "\n*********************************************************************************\nDATA ON PROCESSOR " << processor_id() << '\n';
//  for (unsigned int rank = 0; rank < _app.n_processors(); ++rank)
//    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//      for (std::vector<FeatureData>::iterator l_it = _partial_feature_sets[rank][map_num].begin(); l_it != _partial_feature_sets[rank][map_num].end(); ++l_it)
//      {
//        std::cout << "From Proc: " << rank << "\nItem: " << ++counter << "\nGhosted Entities: ";
//        for (std::set<dof_id_type>::const_iterator it = l_it->_ghosted_ids.begin();
//             it != l_it->_ghosted_ids.end(); ++it)
//          std::cout << *it << " ";
//        std::cout << "\nInterior Entities: ";
//        for (std::set<dof_id_type>::const_iterator it = l_it->_interior_ids.begin();
//             it != l_it->_interior_ids.end(); ++it)
//          std::cout << *it << " ";
//
//        std::cout << "\nVar_idx: " << l_it->_var_idx;
//        std::cout << "\nMax: " << l_it->_bbox.max();
//        std::cout << "\nMin: " << l_it->_bbox.min();
//        std::cout << "\nMin Entitity ID: " << l_it->_min_feature_id;
//        std::cout << "\n\n";
//      }


//  bool start_next_set = true;
//  bool has_data_to_save = false;
//
//  unsigned int curr_set_length = 0;
//  std::set<dof_id_type> curr_set;
//  unsigned int curr_var_idx = std::numeric_limits<unsigned int>::max();
//
//  _region_to_var_idx.clear();
//  for (unsigned int i = 0; i < packed_data.size(); ++i)
//  {
//    if (start_next_set)
//    {
//      if (has_data_to_save)
//      {
//        // See Note at the bottom of this routine
//        _feature_sets[_single_map_mode ? 0 : curr_var_idx].push_back(FeatureData(curr_set, curr_var_idx));
//        _region_to_var_idx.push_back(curr_var_idx);
//        curr_set.clear();
//      }
//
//      // Get the length of the next set
//      curr_set_length = packed_data[i];
//      // Also get the owning variable idx.
//      // Note: We are intentionally advancing "i" here too!
//      curr_var_idx = packed_data[++i];
//    }
//    else
//    {
//      // unpack each bubble
//      curr_set.insert(packed_data[i]);
//      --curr_set_length;
//    }
//
//    start_next_set = !(curr_set_length);
//    has_data_to_save = true;
//  }
//
//  /**
//   * Note: In multi-map mode the var_idx information stored inside of FeatureData is redundant with
//   * the outer index of the _feature_sets data-structure.  We need this information for single-map
//   * mode when we have multiple variables coupled in.
//   */
//  if (has_data_to_save)
//  {
//    _feature_sets[_single_map_mode ? 0 : curr_var_idx].push_back(FeatureData(curr_set, curr_var_idx));
//    _region_to_var_idx.push_back(curr_var_idx);
//  }
//
//  mooseAssert(curr_set_length == 0, "Error in unpacking data");
}

void
FeatureFloodCount::communicateOneList(std::list<FeatureData> & list, unsigned int owner_id, unsigned int map_num)
{
//  Moose::perf_log.push("communicateOneList()", "FeatureFloodCount");
//
//  mooseAssert(!_single_map_mode, "This routine only works in single map mode");
//  std::vector<dof_id_type> packed_data;
//  unsigned int total_size = 0;
//
//  if (owner_id == processor_id())
//  {
//    // First resize our vector to hold our packed data
//    for (std::list<FeatureData>::iterator list_it = list.begin(); list_it != list.end(); ++list_it)
//      total_size += list_it->_ghosted_ids.size() + 1; // The +1 is for the markers between individual sets
//    packed_data.resize(total_size);
//
//    // Now fill in the packed_data data structure
//    unsigned int counter = 0;
//    for (std::list<FeatureData>::iterator list_it = list.begin(); list_it != list.end(); ++list_it)
//    {
//      packed_data[counter++] = list_it->_ghosted_ids.size();
//      for (std::set<dof_id_type>::iterator set_it = list_it->_ghosted_ids.begin(); set_it != list_it->_ghosted_ids.end(); ++set_it)
//        packed_data[counter++] = *set_it;
//    }
//  }
//
//  _communicator.broadcast(total_size, owner_id);
//  packed_data.resize(total_size);
//  _communicator.broadcast(packed_data, owner_id);
//
//  // Unpack
//  if (owner_id != processor_id())
//  {
//    list.clear();
//
//    bool start_next_set = true;
//    bool has_data_to_save = false;
//
//    unsigned int curr_set_length = 0;
//    std::set<dof_id_type> curr_set;
//
//    for (unsigned int i = 0; i < packed_data.size(); ++i)
//    {
//      if (start_next_set)
//      {
//        if (has_data_to_save)
//        {
//          list.push_back(FeatureData(curr_set, map_num)); // map_num == var_idx in multi_map mode
//          curr_set.clear();
//        }
//
//        // Get the length of the next set
//        curr_set_length = packed_data[i];
//      }
//      else
//      {
//        // unpack each bubble
//        curr_set.insert(packed_data[i]);
//        --curr_set_length;
//      }
//
//      start_next_set = !(curr_set_length);
//      has_data_to_save = true;
//    }
//
//    if (has_data_to_save)
//      list.push_back(FeatureData(curr_set, map_num)); // map_num == var_idx in multi_map mode
//
//    mooseAssert(curr_set_length == 0, "Error in unpacking data");
//  }
//  Moose::perf_log.pop("communicateOneList()", "FeatureFloodCount");
}

void
FeatureFloodCount::mergeSets(bool use_periodic_boundary_info)
{
  Moose::perf_log.push("mergeSets()", "FeatureFloodCount");
  std::set<dof_id_type> set_union;
  std::insert_iterator<std::set<dof_id_type> > set_union_inserter(set_union, set_union.begin());

  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    for (processor_id_type rank1 = 0; rank1 < n_procs; ++rank1)
    {
      for (processor_id_type rank2 = 0; rank2 < n_procs; ++rank2)
      {
        if (rank1 == rank2)
          continue;

        for (std::vector<FeatureData>::iterator it1 = _partial_feature_sets[rank1][map_num].begin(); it1 != _partial_feature_sets[rank1][map_num].end(); /* no increment */)
        {
          if (it1->_merged)
          {
            ++it1;
            continue;
          }

          bool region_merged = false;
          for (std::vector<FeatureData>::iterator it2 = _partial_feature_sets[rank2][map_num].begin(); it2 != _partial_feature_sets[rank2][map_num].end(); ++it2)
          {
            if (it1 != it2 &&                                                                // Make sure that these iterators aren't pointing at the same set
                !it2->_merged &&                                                             // and that it2 is not merged (it1 was already checked)
                it1->_var_idx == it2->_var_idx &&                                            // and that the sets have matching variable indices

                 (it1->_bbox.intersect(it2->_bbox) ||                                        // and either their bounding boxes intersect
                   (use_periodic_boundary_info &&                                            // or if we are merging across periodic nodes
                     setsIntersect(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(), // their periodic node sets overlap
                                   it2->_periodic_nodes.begin(), it2->_periodic_nodes.end())
                   )
                 )
               )
            {
              // Merge these two entity sets
              set_union.clear();
              std::set_union(it1->_ghosted_ids.begin(), it1->_ghosted_ids.end(), it2->_ghosted_ids.begin(), it2->_ghosted_ids.end(), set_union_inserter);
              it1->_ghosted_ids.swap(set_union);
              it2->_ghosted_ids.clear();

              // If we are merging periodic boundaries we'll need to merge those nodes too
              if (use_periodic_boundary_info)
              {
                set_union.clear();
                std::set_union(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(), it2->_periodic_nodes.begin(), it2->_periodic_nodes.end(), set_union_inserter);
                it1->_periodic_nodes.swap(set_union);
                it2->_periodic_nodes.clear();
              }

              // Merge interior nodes (this case rarely occurs so we'll avoid building the intersection if we can avoid it)
              if (!it1->_interior_ids.empty() && !it2->_interior_ids.empty())
              {
                set_union.clear();
                std::set_union(it1->_interior_ids.begin(), it1->_interior_ids.end(), it2->_interior_ids.begin(), it2->_interior_ids.end(), set_union_inserter);
                it1->_interior_ids.swap(set_union);
                it2->_interior_ids.clear();
              }

              // Update the bounding box
              it1->updateBBoxMax(it2->_bbox.max());
              it1->updateBBoxMin(it2->_bbox.min());

              // Update the min feature id
              it1->_min_feature_id = std::min(it1->_min_feature_id, it2->_min_feature_id);

              // Set the flag on the merged set so we don't revisit it again
              it2->_merged = true;

              // Something was merged so we'll need to repeat this loop
              region_merged = true;
            }
          } // it2 loop

          // Don't increment if we had a merge, we need to retry earlier candidates again
          if (!region_merged)
            ++it1;

        } // it1 loop
      } // rank2 loop
    } // rank1 loop
  } // map loop

  // Now consolidate the data structure
  for (processor_id_type rank = 0; rank < n_procs; ++rank)
  {
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      if (rank == 0)
        _feature_sets[map_num].clear();

      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)

        if (!it->_merged)
          _feature_sets[map_num].push_back(*it);
    }
  }

   /**
    * If map_num <= n_processors (normal case), each processor up to map_num will handle one list
    * of nodes and receive the merged nodes from other processors for all other lists.
    */
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//  {
//    unsigned int owner_id = map_num % _app.n_processors();
//    if (_single_map_mode || owner_id == processor_id())
//    {
//      // Get an iterator pointing to the end of the list, we'll reuse it several times in the merge algorithm below
//      std::list<FeatureData>::iterator end = _feature_sets[map_num].end();
//
//      // Next add periodic neighbor information if requested to the FeatureData objects
//      if (use_periodic_boundary_info)
//        for (std::list<FeatureData>::iterator it = _feature_sets[map_num].begin(); it != end; ++it)
//          appendPeriodicNeighborNodes(*it);
//
//      // Finally start our merge loops
//      for (std::list<FeatureData>::iterator it1 = _feature_sets[map_num].begin(); it1 != end; /* No increment */)
//      {
//        bool need_it1_increment = true;
//
//        for (std::list<FeatureData>::iterator it2 = it1; it2 != end; ++it2)
//        {
//          if (it1 != it2 &&                                                               // Make sure that these iterators aren't pointing at the same set
//              it1->_var_idx == it2->_var_idx &&                                           // and that the sets have matching variable indices...
//              (setsIntersect(it1->_ghosted_ids.begin(), it1->_ghosted_ids.end(),            // Do they overlap on the current entity type? OR..
//                             it2->_ghosted_ids.begin(), it2->_ghosted_ids.end()) ||
//                 (use_periodic_boundary_info &&                                           // Are we merging across periodic boundaries? AND
//                 setsIntersect(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(),  // Do they overlap on periodic nodes?
//                               it2->_periodic_nodes.begin(), it2->_periodic_nodes.end())
//                 )
//              )
//            )
//          {
//            // Merge these two entity sets
//            set_union.clear();
//            std::set_union(it1->_ghosted_ids.begin(), it1->_ghosted_ids.end(), it2->_ghosted_ids.begin(), it2->_ghosted_ids.end(), set_union_inserter);
//            // Put the merged set in the latter iterator so that we'll compare earlier sets to it again
//            it2->_ghosted_ids = set_union;
//
//            // If we are merging periodic boundaries we'll need to merge those nodes too
//            if (use_periodic_boundary_info)
//            {
//              set_union.clear();
//              std::set_union(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(), it2->_periodic_nodes.begin(), it2->_periodic_nodes.end(), set_union_inserter);
//              it2->_periodic_nodes = set_union;
//            }
//
//            // Now remove the merged set, the one we didn't update (it1)
//            _feature_sets[map_num].erase(it1++);
//            // don't increment the outer loop since we just deleted it incremented
//            need_it1_increment = false;
//            // break out of the inner loop and move on
//            break;
//          }
//        }
//
//        if (need_it1_increment)
//          ++it1;
//      }
//    }
//  }
//
//  if (!_single_map_mode)
//    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//      // Now communicate this list with all the other processors
//      communicateOneList(_feature_sets[map_num], map_num % _app.n_processors(), map_num);
//
  Moose::perf_log.pop("mergeSets()", "FeatureFloodCount");
}

void
FeatureFloodCount::updateFieldInfo()
{
//  // This variable is only relevant in single map mode
//  _region_to_var_idx.resize(_feature_sets[0].size());
//
//  // Finally update the original bubble map with field data from the merged sets
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//  {
//    _feature_maps[map_num].clear();
//
//    unsigned int counter = 0;
//    for (std::list<FeatureData>::iterator it1 = _feature_sets[map_num].begin(); it1 != _feature_sets[map_num].end(); ++it1)
//    {
//      std::cout << "Size: " << it1->_ghosted_ids.size() << std::endl;
//      for (std::set<dof_id_type>::iterator it2 = it1->_ghosted_ids.begin(); it2 != it1->_ghosted_ids.end(); ++it2)
//      {
//        // Color the bubble map with a unique region
//        _feature_maps[map_num][*it2] = counter;
////        if (_var_index_mode)
////          _var_index_maps[map_num][*it2] = it1->_var_idx;
//      }
////
////      if (_single_map_mode)
////        _region_to_var_idx[counter] = it1->_var_idx;
//      ++counter;
//    }
//
////    std::cerr << "Proc: " << processor_id() << " map_num: " << map_num << " : " << counter << '\n';
//    _region_counts[map_num] = counter;
//  }
}

void
FeatureFloodCount::flood(const DofObject * dof_object, int current_idx, int live_region)
{
  if (dof_object == NULL)
    return;

  // Retrieve the id of the current entity
  dof_id_type entity_id = dof_object->id();

  // Has this entity already been marked? - if so move along
  if (_entities_visited[current_idx].find(entity_id) != _entities_visited[current_idx].end())
    return;

  // Mark this entity as visited
  _entities_visited[current_idx][entity_id] = true;

  // Determine which threshold to use based on whether this is an established region
  Real threshold = (live_region > -1 ? _step_connecting_threshold : _step_threshold);

  // Get the value of the current variable for the current entity
  Number entity_value;
  if (_is_elemental)
  {
    const Elem * elem = static_cast<const Elem *>(dof_object);
    std::vector<Point> centroid(1, elem->centroid());
    _fe_problem.reinitElemPhys(elem, centroid, 0);
    entity_value = _vars[current_idx]->sln()[0];
  }
  else
    entity_value = _vars[current_idx]->getNodalValue(*static_cast<const Node *>(dof_object));

  // This node hasn't been marked, is it in a bubble?  We must respect
  // the user-selected value of _use_less_than_threshold_comparison.
  if (_use_less_than_threshold_comparison && (entity_value < threshold))
    return;

  if (!_use_less_than_threshold_comparison && (entity_value > threshold))
    return;

  // Yay! A bubble -> Mark it!
  unsigned int map_num = _single_map_mode ? 0 : current_idx;
  if (live_region > -1)
    _feature_maps[map_num][entity_id] = live_region;
  else
  {
    _feature_maps[map_num][entity_id] = _region_counts[map_num]++;
    _region_to_var_idx.push_back(current_idx);
  }

  if (_is_elemental)
  {
    const Elem * elem = static_cast<const Elem *>(dof_object);
    std::vector<const Elem *> all_active_neighbors;

    // Loop over all neighbors (at the the same level as the current element)
    for (unsigned int i = 0; i < elem->n_neighbors(); ++i)
    {
      const Elem * neighbor_ancestor = elem->neighbor(i);
      if (neighbor_ancestor)
        // Retrieve only the active neighbors for each side of this element, append them to the list of active neighbors
        neighbor_ancestor->active_family_tree_by_neighbor(all_active_neighbors, elem, false);
    }

    // Loop over all active neighbors
    for (std::vector<const Elem *>::const_iterator neighbor_it = all_active_neighbors.begin(); neighbor_it != all_active_neighbors.end(); ++neighbor_it)
    {
      const Elem * neighbor = *neighbor_it;

      // Only recurse on elems this processor can see
      if (neighbor && neighbor->is_semilocal(processor_id()))
        flood(neighbor, current_idx, _feature_maps[map_num][entity_id]);
    }
  }
  else
  {
    std::vector<const Node *> neighbors;
    MeshTools::find_nodal_neighbors(_mesh.getMesh(), *static_cast<const Node *>(dof_object), _nodes_to_elem_map, neighbors);
    // Flood neighboring nodes that are also above this threshold with recursion
    for (unsigned int i = 0; i < neighbors.size(); ++i)
    {
      // Only recurse on nodes this processor can see
      if (_mesh.isSemiLocal(const_cast<Node *>(neighbors[i])))
        flood(neighbors[i], current_idx, _feature_maps[map_num][entity_id]);
    }
  }
}

void
FeatureFloodCount::appendPeriodicNeighborNodes(FeatureData & data) const
{
  // Using a typedef makes the code easier to understand and avoids repeating information.
  typedef std::multimap<dof_id_type, dof_id_type>::const_iterator IterType;

  if (_is_elemental)
  {
    for (std::set<dof_id_type>::iterator entity_it = data._ghosted_ids.begin(); entity_it != data._ghosted_ids.end(); ++entity_it)
    {
      Elem * elem = _mesh.elem(*entity_it);

      for (unsigned int node_n = 0; node_n < elem->n_nodes(); node_n++)
      {
        std::pair<IterType, IterType> iters = _periodic_node_map.equal_range(elem->node(node_n));

        for (IterType it = iters.first; it != iters.second; ++it)
        {
          data._periodic_nodes.insert(it->first);
          data._periodic_nodes.insert(it->second);
        }
      }
    }
  }
  else
  {
    for (std::set<dof_id_type>::iterator entity_it = data._ghosted_ids.begin(); entity_it != data._ghosted_ids.end(); ++entity_it)
    {
      std::pair<IterType, IterType> iters = _periodic_node_map.equal_range(*entity_it);

      for (IterType it = iters.first; it != iters.second; ++it)
      {
        data._periodic_nodes.insert(it->first);
        data._periodic_nodes.insert(it->second);
      }
    }
  }
}

void
FeatureFloodCount::updateRegionOffsets()
{
  if (_global_numbering)
    // Note: We never need to touch offset zero - it should *always* be zero
    for (unsigned int map_num = 1; map_num < _maps_size; ++map_num)
      _region_offsets[map_num] = _region_offsets[map_num -1] + _region_counts[map_num - 1];
}

void
FeatureFloodCount::inflateBoundingBoxes(Real inflation_amount)
{
  processor_id_type n_procs = _app.n_processors();

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    for (processor_id_type rank = 0; rank < n_procs; ++rank)
      for (std::vector<FeatureData>::iterator it = _partial_feature_sets[rank][map_num].begin();
           it != _partial_feature_sets[rank][map_num].end(); ++it)
        it->inflateBoundingBox(inflation_amount);
}

void
FeatureFloodCount::calculateBubbleVolumes()
{
//  Moose::perf_log.push("calculateBubbleVolume()", "FeatureFloodCount");
//
//  // Figure out which bubbles intersect the boundary if the user has enabled that capability.
//  if (_compute_boundary_intersecting_volume)
//  {
//    // Create a std::set of node IDs which are on the boundary called all_boundary_node_ids.
//    std::set<dof_id_type> all_boundary_node_ids;
//
//    // Iterate over the boundary nodes, putting them into the std::set data structure
//    MooseMesh::bnd_node_iterator
//      boundary_nodes_it  = _mesh.bndNodesBegin(),
//      boundary_nodes_end = _mesh.bndNodesEnd();
//    for (; boundary_nodes_it != boundary_nodes_end; ++boundary_nodes_it)
//    {
//      BndNode * boundary_node = *boundary_nodes_it;
//      all_boundary_node_ids.insert(boundary_node->_node->id());
//    }
//
//    // For each of the _maps_size FeatureData lists, determine if the set
//    // of nodes includes any boundary nodes.
//    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//    {
//      std::list<FeatureData>::iterator
//        bubble_it = _feature_sets[map_num].begin(),
//        bubble_end = _feature_sets[map_num].end();
//
//      // Determine boundary intersection for each FeatureData object
//      for (; bubble_it != bubble_end; ++bubble_it)
//        bubble_it->_intersects_boundary = setsIntersect(all_boundary_node_ids.begin(), all_boundary_node_ids.end(),
//                                                        bubble_it->_ghosted_ids.begin(), bubble_it->_ghosted_ids.end());
//    }
//  }
//
//  // Size our temporary data structure
//  std::vector<std::vector<Real> > bubble_volumes(_maps_size);
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//    bubble_volumes[map_num].resize(_feature_sets[map_num].size());
//
//  // Clear pre-existing values and allocate space to store the volume
//  // of the boundary-intersecting grains for each variable.
//  _total_volume_intersecting_boundary.clear();
//  _total_volume_intersecting_boundary.resize(_maps_size);
//
//  // Loop over the active local elements.  For each variable, and for
//  // each FeatureData object, check whether a majority of the element's
//  // nodes belong to that Bubble, and if so assign the element's full
//  // volume to that bubble.
//  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
//  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
//  {
//    Elem * elem = *el;
//    unsigned int elem_n_nodes = elem->n_nodes();
//    Real curr_volume = elem->volume();
//
//    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//    {
//      std::list<FeatureData>::const_iterator
//        bubble_it = _feature_sets[map_num].begin(),
//        bubble_end = _feature_sets[map_num].end();
//
//      for (unsigned int bubble_counter = 0; bubble_it != bubble_end; ++bubble_it, ++bubble_counter)
//      {
//        // Count the number of nodes on this element which are flooded.
//        unsigned int flooded_nodes = 0;
//        for (unsigned int node = 0; node < elem_n_nodes; ++node)
//        {
//          dof_id_type node_id = elem->node(node);
//          if (bubble_it->_ghosted_ids.find(node_id) != bubble_it->_ghosted_ids.end())
//            ++flooded_nodes;
//        }
//
//        // If a majority of the nodes for this element are flooded,
//        // assign its volume to the current bubble_counter entry.
//        if (flooded_nodes >= elem_n_nodes / 2)
//        {
//          bubble_volumes[map_num][bubble_counter] += curr_volume;
//
//          // If the current bubble also intersects the boundary, also
//          // accumlate the volume into the total volume of bubbles
//          // which intersect the boundary.
//          if (bubble_it->_intersects_boundary)
//            _total_volume_intersecting_boundary[map_num] += curr_volume;
//        }
//      }
//    }
//  }
//
//  // If we're calculating boundary-intersecting volumes, we have to normalize it by the
//  // volume of the entire domain.
//  if (_compute_boundary_intersecting_volume)
//  {
//    // Compute the total area using a bounding box.  FIXME: this
//    // assumes the domain is rectangular and 2D, and is probably a
//    // little expensive so we should only do it once if possible.
//    MeshTools::BoundingBox bbox = MeshTools::bounding_box(_mesh);
//    Real total_volume = (bbox.max()(0)-bbox.min()(0))*(bbox.max()(1)-bbox.min()(1));
//
//    // Sum up the partial boundary grain volume contributions from all processors
//    _communicator.sum(_total_volume_intersecting_boundary);
//
//    // Scale the boundary intersecting grain volumes by the total domain volume
//    for (unsigned int i = 0; i<_total_volume_intersecting_boundary.size(); ++i)
//      _total_volume_intersecting_boundary[i] /= total_volume;
//  }
//
//  // Stick all the partial bubble volumes in one long single vector to be gathered on the root processor
//  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
//    _all_feature_volumes.insert(_all_feature_volumes.end(), bubble_volumes[map_num].begin(), bubble_volumes[map_num].end());
//
//  // do all the sums!
//  _communicator.sum(_all_feature_volumes);
//
//  std::sort(_all_feature_volumes.begin(), _all_feature_volumes.end(), std::greater<Real>());
//
//  Moose::perf_log.pop("calculateBubbleVolume()", "FeatureFloodCount");
}

void
FeatureFloodCount::FeatureData::updateBBoxMin(const Point & min)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _bbox.min()(i) = std::min(_bbox.min()(i), min(i));
}

void
FeatureFloodCount::FeatureData::updateBBoxMax(const Point & max)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _bbox.max()(i) = std::max(_bbox.max()(i), max(i));
}

const std::vector<std::pair<unsigned int, unsigned int> > FeatureFloodCount::_empty;
