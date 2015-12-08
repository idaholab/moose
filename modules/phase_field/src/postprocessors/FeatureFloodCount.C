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
struct string_list_inserter
  : std::iterator<std::output_iterator_tag, T>
{
  explicit string_list_inserter(std::list<T *> & list) : _list(list)
    {
    }

  template <typename T2>
  void operator=(const T2 & value)
    {
      _list.push_back(value);
    }

  string_list_inserter& operator++() {
    return *this;
  }

  string_list_inserter operator++(int) {
    return string_list_inserter(*this);
  }

  // We don't return a reference-to-T here because we don't want to
  // construct one or have any of its methods called.
  string_list_inserter& operator*() { return *this; }

private:
  std::list<T *> & _list;
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


template<> void dataStore(std::ostream & stream, FeatureFloodCount::BubbleData & feature, void * context)
{
  storeHelper(stream, feature._entity_ids, context);
  storeHelper(stream, feature._var_idx, context);
  storeHelper(stream, feature._bbox.min(), context);
  storeHelper(stream, feature._bbox.max(), context);
  storeHelper(stream, feature._min_feature_id, context);
}

template<> void dataLoad(std::istream & stream, FeatureFloodCount::BubbleData & feature, void * context)
{
  loadHelper(stream, feature._entity_ids, context);
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
  params.addParam<bool>("track_memory_usage", false, "Calculate memory usage");
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
    _track_memory(getParam<bool>("track_memory_usage")),
    _compute_boundary_intersecting_volume(getParam<bool>("compute_boundary_intersecting_volume")),
    _is_elemental(getParam<MooseEnum>("flood_entity_type") == "ELEMENTAL" ? true : false)
{
  // Size the data structures to hold the correct number of maps
  _bubble_maps.resize(_maps_size);
  _bubble_sets.resize(_maps_size);
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
    _bubble_maps[map_num].clear();
    _bubble_sets[map_num].clear();
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

  _all_bubble_volumes.clear();

  _bytes_used = 0;

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
  std::string packed_buffer;
//  switch (processor_id())
//  {
//  case 0:
//    for (unsigned int i=0; i<10000; ++i)
//      packed_buffer.push_back('x');
//    break;
////    packed_buffer.assign("zero"); break;
//  case 1:
//    packed_buffer.assign("one"); break;
//  case 2:
//    packed_buffer.assign("two"); break;
//  case 3:
//    packed_buffer.assign("three"); break;
//  case 4:
//    packed_buffer.assign("four"); break;
//  case 5:
//    packed_buffer.assign("five"); break;
//  case 6:
//    packed_buffer.assign("six"); break;
//  default:
//    packed_buffer.assign("some bigger number"); break;
//  }

  std::vector<std::string *> send_buffer(1, &packed_buffer);
  std::list<std::string *> recv_buffer;

  // Exchange data in parallel
  pack(&packed_buffer);

//  if (processor_id() == 0)
//    _communicator.send(1, packed_buffer);
//  else
//  {
//    packed_buffer.clear();
//    _communicator.receive(0, packed_buffer);
//  }

  _communicator.allgather_packed_range((void *)(NULL), send_buffer.begin(), send_buffer.end(),
                                       string_list_inserter<std::string>(recv_buffer));

//  // Print the raw unpacked buffer
//  for (std::list<std::string *>::iterator it = recv_buffer.begin(); it != recv_buffer.end(); ++it)
//    std::cout << **it << '\n';

//  _communicator.allgather(packed_buffer, false);
  unpack(recv_buffer);

  exit(0);


  mergeSets(true);

  // Populate _bubble_maps and _var_index_maps
  updateFieldInfo();

  // Update the region offsets so we can get unique bubble numbers in multimap mode
  updateRegionOffsets();

  // Calculate and out output bubble volume data
  if (_pars.isParamValid("bubble_volume_file"))
  {
    calculateBubbleVolumes();
    std::vector<Real> data;
    data.reserve(_all_bubble_volumes.size() + _total_volume_intersecting_boundary.size() + 2);

    // Insert the current timestep and the simulation time into the data vector
    data.push_back(_fe_problem.timeStep());
    data.push_back(_fe_problem.time());

    // Insert the (sorted) bubble volumes into the data vector
    data.insert(data.end(), _all_bubble_volumes.begin(), _all_bubble_volumes.end());

    // If we are computing the boundary-intersecting volumes, insert
    // those numbers into the normalized boundary-intersecting bubble
    // volumes into the data vector.
    if (_compute_boundary_intersecting_volume)
      data.insert(data.end(), _total_volume_intersecting_boundary.begin(), _total_volume_intersecting_boundary.end());

    // Finally, write the file
    writeCSVFile(getParam<FileName>("bubble_volume_file"), data);
  }

  // Calculate memory usage
  if (_track_memory)
  {
    _bytes_used += calculateUsage();
    _communicator.sum(_bytes_used);
    formatBytesUsed();
  }
}

Real
FeatureFloodCount::getValue()
{
  unsigned int count = 0;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    count += _bubble_sets[map_num].size();

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
    std::map<dof_id_type, int>::const_iterator entity_it = _bubble_maps[var_idx].find(entity_id);

    if (entity_it != _bubble_maps[var_idx].end())
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
FeatureFloodCount::pack(std::string * packed_buffer)
{
//  std::vector<FooBar> foo_bar(1);
//
//  sleep(processor_id());
//
//  if (_app.n_processors() == 2)
//  {
//    if (processor_id() == 0)
//    {
////      foo_bar[0] = new FooBar();
//      foo_bar[0]._entity_ids.insert(5);
//      foo_bar[0]._entity_ids.insert(6);
//      foo_bar[0]._entity_ids.insert(7);
//      foo_bar[0]._var_idx = 2;
//      foo_bar[0]._max = Point(0.5, 2.5, 3.14);
//
//
//      for (unsigned int i = 0; i < foo_bar.size(); ++i)
//      {
//        std::cerr << processor_id() << " Set " << i << ": ";
//        for (std::set<dof_id_type>::const_iterator it = foo_bar[i]._entity_ids.begin(); it != foo_bar[i]._entity_ids.end(); ++it)
//          std::cerr << *it << " ";
//        std::cerr << "\nVar_idx: " << foo_bar[i]._var_idx;
//        std::cerr << "\nMax: " << foo_bar[i]._max;
//        std::cerr << "\n\n";
//      }
//
//      std::ostringstream oss;
//      dataStore(oss, foo_bar, this);
//
//      std::string buf = oss.str();
////      _communicator.send(1, buf);
//    }
//    else
//    {
//      foo_bar.clear();
//      std::string buf;
////      _communicator.receive(0, buf);
//      std::istringstream iss(buf);
//
////      dataLoad(iss, foo_bar, this);
//
//      for (unsigned int i = 0; i < foo_bar.size(); ++i)
//      {
//        std::cerr << processor_id() << " Set " << i << ": ";
//        for (std::set<dof_id_type>::const_iterator it = foo_bar[i]._entity_ids.begin(); it != foo_bar[i]._entity_ids.end(); ++it)
//          std::cerr << *it << " ";
//        std::cerr << "\nVar_idx: " << foo_bar[i]._var_idx;
//        std::cerr << "\nMax: " << foo_bar[i]._max;
//        std::cerr << "\n\n";
//      }
//    }
//  }

  /**
   * We need a data structure that reorganizes the region markings into sets so that we can pack them up
   * in a form to marshall them between processors.  The set of nodes are stored by region_num for the
   * current map_num.
   **/
  std::vector<BubbleData> ghost_data;

  /**
   * This data structure holds just the local processors markings. It's used to prepopulate
   * the bubble_sets data structure for use in the mergeSets routine.
   */
  std::vector<std::set<dof_id_type> > local_data;

//  std::vector<Point> min_points;
//  std::vector<Point> max_points;
//  std::vector<dof_id_type> min_feature_id;

  MeshBase & mesh = _mesh.getMesh();

  std::ostringstream oss;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    ghost_data.clear();

    ghost_data.resize(_region_counts[map_num]);
    local_data.resize(_region_counts[map_num]);
//    min_points.resize(_region_counts[map_num],  std::numeric_limits<Real>::max());
//    max_points.resize(_region_counts[map_num], -std::numeric_limits<Real>::max());
//    min_feature_id.resize(_region_counts[map_num], std::numeric_limits<Real>::max());

//    unsigned int ghost_counter = 0;

    std::map<dof_id_type, int>::const_iterator b_end = _bubble_maps[map_num].end();
    // Reorganize the data by values
    for (std::map<dof_id_type, int>::const_iterator b_it = _bubble_maps[map_num].begin(); b_it != b_end; ++b_it)
    {
      // Local variables for clarity
      dof_id_type entity_id = b_it->first;
      int partial_feature_num = b_it->second;

      /**
       * Neighboring processors only need enough information to stitch together the regions
       * we'll only pack information that's on the processor boundaries.
       */
      if (_ghosted_entity_ids.find(entity_id) != _ghosted_entity_ids.end())
      {
        ghost_data[partial_feature_num]._entity_ids.insert(entity_id);
//        ++ghost_counter;
      }

      // Now retrieve the location of this entity for determining the bounding box region
      const Point & entity_point = _is_elemental ? mesh.elem(entity_id)->centroid() : mesh.node(entity_id);

      ghost_data[partial_feature_num].updateBBoxMin(entity_point);
      ghost_data[partial_feature_num].updateBBoxMax(entity_point);

      // Finally save off the min entity id present in the feature to uniquely identify the feature regardless of n_procs
      ghost_data[partial_feature_num]._min_feature_id = std::min(ghost_data[partial_feature_num]._min_feature_id, entity_id);

      /**
       * However we still need to save all of the local data for use in merging
       * and field update routines.
       */
      local_data[(b_it->second)].insert(b_it->first);
    }

    // Save the variable index
    for (unsigned int i = 0; i < _region_counts[map_num]; ++i)
      ghost_data[i]._var_idx = _region_to_var_idx[i];


//    std::cout << oss.str() << '\n';


    /**
     * We'll save the local data immediately two our bubble_sets data structure. It doesn't
     * need to be communicated.
     */
//    for (unsigned int i = 0; i < _region_counts[map_num]; ++i)
//      _bubble_sets[_single_map_mode ? 0 : map_num].push_back(BubbleData(local_data[i], _region_to_var_idx[i]));


    /****************************************************************************************************
     * TODO: I believe this routine is properly packaging up entities on ghosted regions now. Those
     * regions will be unpacked and merged with neighboring processors. What we need to do know is
     * create a seperate data structure for merging local only bubbles and figure out how to count them.
     *
     * It seems to me that if a bubble is "completely" within the local region not only will it
     * not be merged with any other set, but it'll only be counted on that single processor. This is not
     * quite true when using periodic boundaries.
     */



    /**
     * The size of the packed data structure should be the sum of all of the following:
     * total number of marked entities in ghost regions
     * the owning variable index for the current region
     * the number of unique regions.
     *
     * We will pack the data into a series of groups representing each unique region
     * the entities for each group will be proceeded by the number of entities and the owning
     * variable for that group.
     * @verbatim
     * [ <i_nodes> <var_idx> <n_0> <n_1> ... <n_i> <j_nodes> <var_idx> <n_0> <n_1> ... <n_j> ]
     * @endverbatim
     */

    // Note the _region_counts[mar_num]*2 takes into account the number of nodes and the variable index for each region
//    std::vector<dof_id_type> partial_packed_data;
//    partial_packed_data.reserve(ghost_counter + _region_counts[map_num]*2);
//
    // Now pack it up
    dataStore(oss, ghost_data, this);
    packed_buffer->assign(oss.str());
  }



//    for (unsigned int i = 0; i < _region_counts[map_num]; ++i)
//    {
//      // Skip over the empty sets
//      if (ghost_data[i].empty())
//        continue;
//
//      partial_packed_data.push_back(ghost_data[i].size());              // The number of nodes in the current region
//
//      if (_single_map_mode)
//      {
//        mooseAssert(i < _region_to_var_idx.size(), "Index out of bounds in FeatureFloodCounter");
//        partial_packed_data.push_back(_region_to_var_idx[i]);           // The variable owning this bubble
//      }
//      else
//        partial_packed_data.push_back(map_num);                         // The variable owning this bubble
//
//      std::set<dof_id_type>::iterator end = ghost_data[i].end();
//      for (std::set<dof_id_type>::iterator it = ghost_data[i].begin(); it != end; ++it)
//        partial_packed_data.push_back(*it);                             // The individual entity ids
//    }
//
//    packed_data.insert(packed_data.end(), partial_packed_data.begin(), partial_packed_data.end());
//  }




}

void
FeatureFloodCount::unpack(std::list<std::string *> & packed_buffer_list)
{
  std::list<BubbleData> ghost_data;

  for (std::list<std::string *>::iterator it = packed_buffer_list.begin(); it != packed_buffer_list.end(); ++it)
  {
    std::istringstream iss(**it);
    dataLoad(iss, ghost_data, this);
  }





//  std::list<BubbleData> ghost_data;
//
//  std::istringstream iss(packed_data);
//  dataLoad(iss, ghost_data, this);
//
  unsigned int counter = 0;
//  if (processor_id() == 1)
//  {
    std::list<BubbleData>::iterator l_end = ghost_data.end();

    for (std::list<BubbleData>::iterator l_it = ghost_data.begin(); l_it != l_end; ++l_it)
    {
      std::cout << "Proc: " << processor_id() << "\nItem: " << ++counter << "\nGhosted Entities: ";
      for (std::set<dof_id_type>::const_iterator it = l_it->_entity_ids.begin();
           it != l_it->_entity_ids.end(); ++it)
        std::cout << *it << " ";
      std::cout << "\nVar_idx: " << l_it->_var_idx;
      std::cout << "\nMax: " << l_it->_bbox.max();
      std::cout << "\nMin: " << l_it->_bbox.min();
      std::cout << "\nMin Entitity ID: " << l_it->_min_feature_id;
      std::cout << "\n\n";
    }
//  }




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
//        _bubble_sets[_single_map_mode ? 0 : curr_var_idx].push_back(BubbleData(curr_set, curr_var_idx));
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
//   * Note: In multi-map mode the var_idx information stored inside of BubbleData is redundant with
//   * the outer index of the _bubble_sets data-structure.  We need this information for single-map
//   * mode when we have multiple variables coupled in.
//   */
//  if (has_data_to_save)
//  {
//    _bubble_sets[_single_map_mode ? 0 : curr_var_idx].push_back(BubbleData(curr_set, curr_var_idx));
//    _region_to_var_idx.push_back(curr_var_idx);
//  }
//
//  mooseAssert(curr_set_length == 0, "Error in unpacking data");
}

void
FeatureFloodCount::communicateOneList(std::list<BubbleData> & list, unsigned int owner_id, unsigned int map_num)
{
  Moose::perf_log.push("communicateOneList()", "FeatureFloodCount");

  mooseAssert(!_single_map_mode, "This routine only works in single map mode");
  std::vector<dof_id_type> packed_data;
  unsigned int total_size = 0;

  if (owner_id == processor_id())
  {
    // First resize our vector to hold our packed data
    for (std::list<BubbleData>::iterator list_it = list.begin(); list_it != list.end(); ++list_it)
      total_size += list_it->_entity_ids.size() + 1; // The +1 is for the markers between individual sets
    packed_data.resize(total_size);

    // Now fill in the packed_data data structure
    unsigned int counter = 0;
    for (std::list<BubbleData>::iterator list_it = list.begin(); list_it != list.end(); ++list_it)
    {
      packed_data[counter++] = list_it->_entity_ids.size();
      for (std::set<dof_id_type>::iterator set_it = list_it->_entity_ids.begin(); set_it != list_it->_entity_ids.end(); ++set_it)
        packed_data[counter++] = *set_it;
    }
  }

  _communicator.broadcast(total_size, owner_id);
  packed_data.resize(total_size);
  _communicator.broadcast(packed_data, owner_id);

  // Unpack
  if (owner_id != processor_id())
  {
    list.clear();

    bool start_next_set = true;
    bool has_data_to_save = false;

    unsigned int curr_set_length = 0;
    std::set<dof_id_type> curr_set;

    for (unsigned int i = 0; i < packed_data.size(); ++i)
    {
      if (start_next_set)
      {
        if (has_data_to_save)
        {
          list.push_back(BubbleData(curr_set, map_num)); // map_num == var_idx in multi_map mode
          curr_set.clear();
        }

        // Get the length of the next set
        curr_set_length = packed_data[i];
      }
      else
      {
        // unpack each bubble
        curr_set.insert(packed_data[i]);
        --curr_set_length;
      }

      start_next_set = !(curr_set_length);
      has_data_to_save = true;
    }

    if (has_data_to_save)
      list.push_back(BubbleData(curr_set, map_num)); // map_num == var_idx in multi_map mode

    mooseAssert(curr_set_length == 0, "Error in unpacking data");
  }
  Moose::perf_log.pop("communicateOneList()", "FeatureFloodCount");
}

void
FeatureFloodCount::mergeSets(bool use_periodic_boundary_info)
{
  Moose::perf_log.push("mergeSets()", "FeatureFloodCount");
  std::set<dof_id_type> set_union;
  std::insert_iterator<std::set<dof_id_type> > set_union_inserter(set_union, set_union.begin());

  /**
   * If map_num <= n_processors (normal case), each processor up to map_num will handle one list
   * of nodes and receive the merged nodes from other processors for all other lists.
   */
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    unsigned int owner_id = map_num % _app.n_processors();
    if (_single_map_mode || owner_id == processor_id())
    {
      // Get an iterator pointing to the end of the list, we'll reuse it several times in the merge algorithm below
      std::list<BubbleData>::iterator end = _bubble_sets[map_num].end();

      // Next add periodic neighbor information if requested to the BubbleData objects
      if (use_periodic_boundary_info)
        for (std::list<BubbleData>::iterator it = _bubble_sets[map_num].begin(); it != end; ++it)
          appendPeriodicNeighborNodes(*it);

      // Finally start our merge loops
      for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != end; /* No increment */)
      {
        bool need_it1_increment = true;

        for (std::list<BubbleData>::iterator it2 = it1; it2 != end; ++it2)
        {
          if (it1 != it2 &&                                                               // Make sure that these iterators aren't pointing at the same set
              it1->_var_idx == it2->_var_idx &&                                           // and that the sets have matching variable indices...
              (setsIntersect(it1->_entity_ids.begin(), it1->_entity_ids.end(),            // Do they overlap on the current entity type? OR..
                             it2->_entity_ids.begin(), it2->_entity_ids.end()) ||
                 (use_periodic_boundary_info &&                                           // Are we merging across periodic boundaries? AND
                 setsIntersect(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(),  // Do they overlap on periodic nodes?
                               it2->_periodic_nodes.begin(), it2->_periodic_nodes.end())
                 )
              )
            )
          {
            // Merge these two entity sets
            set_union.clear();
            std::set_union(it1->_entity_ids.begin(), it1->_entity_ids.end(), it2->_entity_ids.begin(), it2->_entity_ids.end(), set_union_inserter);
            // Put the merged set in the latter iterator so that we'll compare earlier sets to it again
            it2->_entity_ids = set_union;

            // If we are merging periodic boundaries we'll need to merge those nodes too
            if (use_periodic_boundary_info)
            {
              set_union.clear();
              std::set_union(it1->_periodic_nodes.begin(), it1->_periodic_nodes.end(), it2->_periodic_nodes.begin(), it2->_periodic_nodes.end(), set_union_inserter);
              it2->_periodic_nodes = set_union;
            }

            // Now remove the merged set, the one we didn't update (it1)
            _bubble_sets[map_num].erase(it1++);
            // don't increment the outer loop since we just deleted it incremented
            need_it1_increment = false;
            // break out of the inner loop and move on
            break;
          }
        }

        if (need_it1_increment)
          ++it1;
      }
    }
  }

  if (!_single_map_mode)
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
      // Now communicate this list with all the other processors
      communicateOneList(_bubble_sets[map_num], map_num % _app.n_processors(), map_num);

  Moose::perf_log.pop("mergeSets()", "FeatureFloodCount");
}

void
FeatureFloodCount::updateFieldInfo()
{
  // This variable is only relevant in single map mode
  _region_to_var_idx.resize(_bubble_sets[0].size());

  // Finally update the original bubble map with field data from the merged sets
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    _bubble_maps[map_num].clear();

    unsigned int counter = 0;
    for (std::list<BubbleData>::iterator it1 = _bubble_sets[map_num].begin(); it1 != _bubble_sets[map_num].end(); ++it1)
    {
      std::cout << "Size: " << it1->_entity_ids.size() << std::endl;
      for (std::set<dof_id_type>::iterator it2 = it1->_entity_ids.begin(); it2 != it1->_entity_ids.end(); ++it2)
      {
        // Color the bubble map with a unique region
        _bubble_maps[map_num][*it2] = counter;
//        if (_var_index_mode)
//          _var_index_maps[map_num][*it2] = it1->_var_idx;
      }
//
//      if (_single_map_mode)
//        _region_to_var_idx[counter] = it1->_var_idx;
      ++counter;
    }

//    std::cerr << "Proc: " << processor_id() << " map_num: " << map_num << " : " << counter << '\n';
    _region_counts[map_num] = counter;
  }
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
    _bubble_maps[map_num][entity_id] = live_region;
  else
  {
    _bubble_maps[map_num][entity_id] = _region_counts[map_num]++;
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
        flood(neighbor, current_idx, _bubble_maps[map_num][entity_id]);
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
        flood(neighbors[i], current_idx, _bubble_maps[map_num][entity_id]);
    }
  }
}

void
FeatureFloodCount::appendPeriodicNeighborNodes(BubbleData & data) const
{
  // Using a typedef makes the code easier to understand and avoids repeating information.
  typedef std::multimap<dof_id_type, dof_id_type>::const_iterator IterType;

  if (_is_elemental)
  {
    for (std::set<dof_id_type>::iterator entity_it = data._entity_ids.begin(); entity_it != data._entity_ids.end(); ++entity_it)
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
    for (std::set<dof_id_type>::iterator entity_it = data._entity_ids.begin(); entity_it != data._entity_ids.end(); ++entity_it)
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
FeatureFloodCount::calculateBubbleVolumes()
{
  Moose::perf_log.push("calculateBubbleVolume()", "FeatureFloodCount");

  // Figure out which bubbles intersect the boundary if the user has enabled that capability.
  if (_compute_boundary_intersecting_volume)
  {
    // Create a std::set of node IDs which are on the boundary called all_boundary_node_ids.
    std::set<dof_id_type> all_boundary_node_ids;

    // Iterate over the boundary nodes, putting them into the std::set data structure
    MooseMesh::bnd_node_iterator
      boundary_nodes_it  = _mesh.bndNodesBegin(),
      boundary_nodes_end = _mesh.bndNodesEnd();
    for (; boundary_nodes_it != boundary_nodes_end; ++boundary_nodes_it)
    {
      BndNode * boundary_node = *boundary_nodes_it;
      all_boundary_node_ids.insert(boundary_node->_node->id());
    }

    // For each of the _maps_size BubbleData lists, determine if the set
    // of nodes includes any boundary nodes.
    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::list<BubbleData>::iterator
        bubble_it = _bubble_sets[map_num].begin(),
        bubble_end = _bubble_sets[map_num].end();

      // Determine boundary intersection for each BubbleData object
      for (; bubble_it != bubble_end; ++bubble_it)
        bubble_it->_intersects_boundary = setsIntersect(all_boundary_node_ids.begin(), all_boundary_node_ids.end(),
                                                        bubble_it->_entity_ids.begin(), bubble_it->_entity_ids.end());
    }
  }

  // Size our temporary data structure
  std::vector<std::vector<Real> > bubble_volumes(_maps_size);
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    bubble_volumes[map_num].resize(_bubble_sets[map_num].size());

  // Clear pre-existing values and allocate space to store the volume
  // of the boundary-intersecting grains for each variable.
  _total_volume_intersecting_boundary.clear();
  _total_volume_intersecting_boundary.resize(_maps_size);

  // Loop over the active local elements.  For each variable, and for
  // each BubbleData object, check whether a majority of the element's
  // nodes belong to that Bubble, and if so assign the element's full
  // volume to that bubble.
  const MeshBase::const_element_iterator el_end = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator el = _mesh.getMesh().active_local_elements_begin(); el != el_end; ++el)
  {
    Elem * elem = *el;
    unsigned int elem_n_nodes = elem->n_nodes();
    Real curr_volume = elem->volume();

    for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    {
      std::list<BubbleData>::const_iterator
        bubble_it = _bubble_sets[map_num].begin(),
        bubble_end = _bubble_sets[map_num].end();

      for (unsigned int bubble_counter = 0; bubble_it != bubble_end; ++bubble_it, ++bubble_counter)
      {
        // Count the number of nodes on this element which are flooded.
        unsigned int flooded_nodes = 0;
        for (unsigned int node = 0; node < elem_n_nodes; ++node)
        {
          dof_id_type node_id = elem->node(node);
          if (bubble_it->_entity_ids.find(node_id) != bubble_it->_entity_ids.end())
            ++flooded_nodes;
        }

        // If a majority of the nodes for this element are flooded,
        // assign its volume to the current bubble_counter entry.
        if (flooded_nodes >= elem_n_nodes / 2)
        {
          bubble_volumes[map_num][bubble_counter] += curr_volume;

          // If the current bubble also intersects the boundary, also
          // accumlate the volume into the total volume of bubbles
          // which intersect the boundary.
          if (bubble_it->_intersects_boundary)
            _total_volume_intersecting_boundary[map_num] += curr_volume;
        }
      }
    }
  }

  // If we're calculating boundary-intersecting volumes, we have to normalize it by the
  // volume of the entire domain.
  if (_compute_boundary_intersecting_volume)
  {
    // Compute the total area using a bounding box.  FIXME: this
    // assumes the domain is rectangular and 2D, and is probably a
    // little expensive so we should only do it once if possible.
    MeshTools::BoundingBox bbox = MeshTools::bounding_box(_mesh);
    Real total_volume = (bbox.max()(0)-bbox.min()(0))*(bbox.max()(1)-bbox.min()(1));

    // Sum up the partial boundary grain volume contributions from all processors
    _communicator.sum(_total_volume_intersecting_boundary);

    // Scale the boundary intersecting grain volumes by the total domain volume
    for (unsigned int i = 0; i<_total_volume_intersecting_boundary.size(); ++i)
      _total_volume_intersecting_boundary[i] /= total_volume;
  }

  // Stick all the partial bubble volumes in one long single vector to be gathered on the root processor
  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
    _all_bubble_volumes.insert(_all_bubble_volumes.end(), bubble_volumes[map_num].begin(), bubble_volumes[map_num].end());

  // do all the sums!
  _communicator.sum(_all_bubble_volumes);

  std::sort(_all_bubble_volumes.begin(), _all_bubble_volumes.end(), std::greater<Real>());

  Moose::perf_log.pop("calculateBubbleVolume()", "FeatureFloodCount");
}

template<>
unsigned long
FeatureFloodCount::bytesHelper(std::list<BubbleData> container)
{
  unsigned long bytes = 0;
  for (std::list<BubbleData>::iterator it = container.begin(); it != container.end(); ++it)
    bytes += bytesHelper(it->_entity_ids) + sizeof(it->_var_idx);
  return bytes;
}

unsigned long
FeatureFloodCount::calculateUsage() const
{
  unsigned long bytes = 0;

  for (unsigned int map_num = 0; map_num < _maps_size; ++map_num)
  {
    bytes += bytesHelper(_entities_visited[map_num]);
    bytes += bytesHelper(_bubble_maps[map_num]);

    if (_var_index_mode)
      bytes += bytesHelper(_var_index_maps[map_num]);

    bytes += bytesHelper(_bubble_sets[map_num]);
  }

  bytes += sizeof(unsigned int) * _region_counts.size();
  bytes += sizeof(unsigned int) * _region_to_var_idx.size();
  bytes += sizeof(unsigned int) * _region_offsets.size();

  bytes += bytesHelper(_periodic_node_map);
  bytes += bytesHelper(_file_handles);

  bytes += sizeof(Real) * _all_bubble_volumes.size();

  // Not counted: _nodes_to_elem_map

  return bytes;
}

void
FeatureFloodCount::formatBytesUsed() const
{
  std::stringstream oss;
  oss.precision(1);
  oss << std::fixed;
  if (_bytes_used >= 1<<30)
    oss << name() << " Memory Used: " << _bytes_used / Real(1<<30) << " GB\n";
  else if (_bytes_used >= 1<<20)
    oss << name() << " Memory Used: " << _bytes_used / Real(1<<20) << " MB\n";
  else if (_bytes_used >= 1<<10)
    oss << name() << " Memory Used: " << _bytes_used / Real(1<<10) << " KB\n";
  else
    oss << name() << " Memory Used: " << _bytes_used << " Bytes\n";
  _console << oss.str() << std::endl;
}

void
FeatureFloodCount::BubbleData::updateBBoxMin(const Point & min)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _bbox.min()(i) = std::min(_bbox.min()(i), min(i));
}

void
FeatureFloodCount::BubbleData::updateBBoxMax(const Point & max)
{
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _bbox.max()(i) = std::max(_bbox.max()(i), max(i));
}

const std::vector<std::pair<unsigned int, unsigned int> > FeatureFloodCount::_empty;
