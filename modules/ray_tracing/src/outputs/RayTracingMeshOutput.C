//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// TODO: remove ignore warnings once std::tuple<> is instantiated as a StandardType
// Using push_parallel_vector_data with std::tuple<> leads to a -Wextra error
// https://github.com/libMesh/TIMPI/issues/52
#include "libmesh/ignore_warnings.h"
#include "RayTracingMeshOutput.h"
#include "libmesh/restore_warnings.h"

// Local Includes
#include "RayTracingStudy.h"
#include "TraceData.h"

// libMesh includes
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/remote_elem.h"

using namespace libMesh;

InputParameters
RayTracingMeshOutput::validParams()
{
  InputParameters params = FileOutput::validParams();

  params.addRequiredParam<UserObjectName>("study", "The RayTracingStudy to get the segments from");

  params.addParam<bool>("output_data", false, "Whether or not to also output the Ray's data");
  params.addParam<std::vector<std::string>>("output_data_names",
                                            "The names of specific data to output");
  params.addParam<bool>("output_data_nodal",
                        false,
                        "Whether or not to output the Ray's data in a nodal sense, in which the "
                        "data is interpolated linearly across segments");

  params.addParam<bool>(
      "output_aux_data", false, "Whether or not to also output the Ray's aux data");
  params.addParam<std::vector<std::string>>("output_aux_data_names",
                                            "The names of specific aux data to output");

  MultiMooseEnum props("ray_id intersections pid processor_crossings trajectory_changes",
                       "ray_id intersections");
  params.addParam<MultiMooseEnum>("output_properties", props, "Which Ray properties to output");

  return params;
}

RayTracingMeshOutput::RayTracingMeshOutput(const InputParameters & params)
  : FileOutput(params),
    UserObjectInterface(this),
    _study(getUserObject<RayTracingStudy>("study")),
    _output_data(getParam<bool>("output_data")),
    _output_data_names(isParamValid("output_data_names")
                           ? &getParam<std::vector<std::string>>("output_data_names")
                           : nullptr),
    _output_data_nodal(getParam<bool>("output_data_nodal")),
    _output_aux_data(getParam<bool>("output_aux_data")),
    _output_aux_data_names(isParamValid("output_aux_data_names")
                               ? &getParam<std::vector<std::string>>("output_aux_data_names")
                               : nullptr),
    _ray_id_var(invalid_uint),
    _intersections_var(invalid_uint),
    _pid_var(invalid_uint),
    _processor_crossings_var(invalid_uint),
    _trajectory_changes_var(invalid_uint),
    _segmented_rays(false)
{
  if ((_output_data || _output_data_names) && !_study.dataOnCacheTraces())
    mooseError("In order to output Ray data in output '",
               name(),
               "', the RayTracingStudy '",
               _study.name(),
               "' must set data_on_cache_traces = true");
  if ((_output_aux_data || _output_aux_data_names) && !_study.auxDataOnCacheTraces())
    mooseError("In order to output Ray aux data in output '",
               name(),
               "', the RayTracingStudy '",
               _study.name(),
               "' must set aux_data_on_cache_traces = true");
  if (_output_data_nodal && !_output_data && !_output_data_names)
    paramError("output_data_nodal",
               "Cannot be used unless there is data to output; in addition set either "
               "'output_data' or 'output_data_names");
  if (_output_data_nodal && !_study.segmentsOnCacheTraces())
    paramError("output_data_nodal", "Not supported when study segments_on_cache_traces = false");
  if (_output_data && _output_data_names)
    paramError("output_data", "Cannot be used in addition to 'output_data_names'; choose one");
  if (_output_aux_data && _output_aux_data_names)
    paramError("output_aux_data",
               "Cannot be used in addition to 'output_aux_data_names'; choose one");
}

std::string
RayTracingMeshOutput::filename()
{
  // Append the .e extension on the base file name
  std::ostringstream output;
  output << _file_base << ".e";

  // Add the _000x extension to the file
  if (_file_num > 0)
    output << "-s" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // Return the filename
  return output.str();
}

void
RayTracingMeshOutput::output()
{
  // Do we even have any traces?
  auto num_segments = _study.getCachedTraces().size();
  _communicator.sum(num_segments);
  if (!num_segments)
    mooseError("No cached trace segments were found in the study '", _study.name(), "'.");

  // Build the _inflated_neighbor_bboxes
  buildBoundingBoxes();

  // Build the _segment_mesh
  buildSegmentMesh();

  // Setup the system to store the Ray field data
  setupEquationSystem();

  // Fill the field data
  fillFields();

  // And output
  outputMesh();

  // Done with these
  // We don't necessarily need to create a new mesh every time, but it's easier than
  // checking if the Rays have changed from last time we built a mesh
  _es = nullptr;
  _sys = nullptr;
  _segment_mesh = nullptr;
}

void
RayTracingMeshOutput::buildIDMap()
{
  TIME_SECTION("buildIDMap", 3, "Building RayTracing ID Map");

  // Build the maximum number of nodes required to represent each one of my local Rays
  std::map<RayID, dof_id_type> local_ray_needed_nodes;
  for (const auto & trace_data : _study.getCachedTraces())
  {
    const auto find = local_ray_needed_nodes.find(trace_data._ray_id);
    if (find == local_ray_needed_nodes.end())
      local_ray_needed_nodes[trace_data._ray_id] = neededNodes(trace_data);
    else
    {
      auto & value = find->second;
      value = std::max(value, neededNodes(trace_data));
    }
  }

  // Fill all of my local Ray maxima to be sent to processor 0
  std::map<processor_id_type, std::vector<std::pair<RayID, dof_id_type>>> send_needed_nodes;
  if (local_ray_needed_nodes.size())
  {
    auto & root_entry = send_needed_nodes[0];
    for (const auto & id_nodes_pair : local_ray_needed_nodes)
      root_entry.emplace_back(id_nodes_pair);
  }

  // The global map of ray -> required nodes needed to be filled on processor 0
  std::map<RayID, dof_id_type> global_needed_nodes;
  // Keep track of what Ray IDs we received from what processors so we know who to send back to
  std::unordered_map<processor_id_type, std::set<RayID>> pid_received_ids;
  // Take the required nodes for each Ray and determine on processor 0 the global maximum
  // nodes needed to represent each Ray
  const auto append_global_max =
      [&global_needed_nodes, &pid_received_ids](
          processor_id_type pid, const std::vector<std::pair<RayID, dof_id_type>> & pairs)
  {
    auto & pid_received_entry = pid_received_ids[pid];
    for (const auto & id_max_pair : pairs)
    {
      const RayID ray_id = id_max_pair.first;
      const dof_id_type max = id_max_pair.second;

      const auto find = global_needed_nodes.find(ray_id);
      if (find == global_needed_nodes.end())
        global_needed_nodes.emplace(ray_id, max);
      else
      {
        auto & current_max = find->second;
        current_max = std::max(current_max, max);
      }

      pid_received_entry.insert(ray_id);
    }
  };
  Parallel::push_parallel_vector_data(comm(), send_needed_nodes, append_global_max);

  // Decide on the starting representative starting node and elem ID for each Ray
  std::map<RayID, std::pair<dof_id_type, dof_id_type>> global_ids;
  dof_id_type current_node_id = 0;
  dof_id_type current_elem_id = 0;
  for (auto & pair : global_needed_nodes)
  {
    const RayID ray_id = pair.first;
    const dof_id_type max_nodes = pair.second;

    global_ids.emplace(ray_id, std::make_pair(current_node_id, current_elem_id));

    current_node_id += max_nodes;
    if (max_nodes > 1)
      current_elem_id += max_nodes - 1;
    else // stationary
      current_elem_id += 1;
  }

  // Share the max node IDs so that we have a starting point for unique IDs for elems
  _max_node_id = current_node_id;
  comm().max(_max_node_id);

  // Fill the starting ID information to each processor that needs it from processor 0
  std::unordered_map<processor_id_type, std::vector<std::tuple<RayID, dof_id_type, dof_id_type>>>
      send_ids;
  for (auto & pid_ids_pair : pid_received_ids)
  {
    const processor_id_type pid = pid_ids_pair.first;
    const std::set<RayID> & ray_ids = pid_ids_pair.second;

    auto & pid_send = send_ids[pid];
    for (const RayID ray_id : ray_ids)
    {
      const auto & global_entry = global_ids.at(ray_id);
      const dof_id_type node_id = global_entry.first;
      const dof_id_type elem_id = global_entry.second;
      pid_send.emplace_back(ray_id, node_id, elem_id);
    }
  }

  // Take the starting ID information from processor 0 and store it locally
  const auto append_ids =
      [&](processor_id_type,
          const std::vector<std::tuple<RayID, dof_id_type, dof_id_type>> & tuples)
  {
    for (const auto & tuple : tuples)
    {
      const RayID ray_id = std::get<0>(tuple);
      const dof_id_type node_id = std::get<1>(tuple);
      const dof_id_type elem_id = std::get<2>(tuple);
      _ray_starting_id_map.emplace(ray_id, std::make_pair(node_id, elem_id));
    }
  };

  _ray_starting_id_map.clear();
  Parallel::push_parallel_vector_data(comm(), send_ids, append_ids);
}

void
RayTracingMeshOutput::buildSegmentMesh()
{
  TIME_SECTION("buildSegmentMesh", 3, "Building RayTracing Mesh Output");

  _segmented_rays = false;

  // Tally nodes and elems for the local mesh ahead of time so we can reserve
  // Each segment requires an element, and we need one more node than elems
  dof_id_type num_nodes = 0;
  dof_id_type num_elems = 0;
  for (const auto & entry : _study.getCachedTraces())
  {
    const auto num_segments = entry.numSegments();
    num_nodes += num_segments + 1;
    if (num_segments >= 1)
    {
      _segmented_rays = true;
      num_elems += num_segments;
    }
    else if (entry.stationary())
      num_elems += 1;
  }

  comm().max(_segmented_rays);

  // Build the segment mesh
  mooseAssert(!_segment_mesh, "Not cleared");
  if (_communicator.size() == 1)
    _segment_mesh = std::make_unique<ReplicatedMesh>(_communicator, _mesh_ptr->dimension());
  else
  {
    _segment_mesh = std::make_unique<DistributedMesh>(_communicator, _mesh_ptr->dimension());
    _segment_mesh->set_distributed();
  }
  // We set neighbor links
  _segment_mesh->allow_find_neighbors(false);
  // Don't renumber so that we can remain consistent between processor counts
  _segment_mesh->allow_renumbering(false);
  // As we're building segments for just this partiton, we're partitioning on our own
  _segment_mesh->skip_partitioning(true);
  // Reserve nodes and elems
  _segment_mesh->reserve_nodes(num_nodes);
  _segment_mesh->reserve_elem(num_elems);

  buildIDMap();

  // Decide on a starting ID for each processor
  // Build a mesh segment for each local Ray segment
  // Each one of these objects represents a single Ray's path through this processor
  for (const auto & trace_data : _study.getCachedTraces())
  {
    // If we have no segments, this is a trace that skimmed the corner of a processor boundary and
    // didn't contribute anything
    if (trace_data.numSegments() == 0 && !trace_data.stationary())
      continue;

    dof_id_type node_id, elem_id;
    startingIDs(trace_data, node_id, elem_id);

    // Add the start point
    mooseAssert(!_segment_mesh->query_node_ptr(node_id), "Node already exists");
    Node * last_node =
        _segment_mesh->add_point(trace_data._point_data[0]._point, node_id, processor_id());
    last_node->set_unique_id(node_id++);

    // Stationary, add a NodeElem
    if (trace_data.stationary())
    {
      mooseAssert(!_segment_mesh->query_elem_ptr(elem_id), "Elem already exists");
      auto elem = _segment_mesh->add_elem(Elem::build_with_id(NODEELEM, elem_id));
      elem->processor_id(processor_id());
      elem->set_unique_id(_max_node_id + elem_id++);
      elem->set_node(0) = last_node;
    }
    // Not stationary; add a point and element for each segment
    else
    {
      Elem * last_elem = nullptr;

      for (std::size_t i = 1; i < trace_data._point_data.size(); ++i)
      {
        const auto & point = trace_data._point_data[i]._point;

        // Add next point on the trace
        mooseAssert(!_segment_mesh->query_node_ptr(node_id), "Node already exists");
        Node * node = _segment_mesh->add_point(point, node_id, processor_id());
        node->set_unique_id(node_id++);

        // Build a segment from this point to the last
        mooseAssert(!_segment_mesh->query_elem_ptr(elem_id), "Elem already exists");
        Elem * elem = _segment_mesh->add_elem(Elem::build_with_id(EDGE2, elem_id));
        elem->processor_id(processor_id());
        elem->set_unique_id(_max_node_id + elem_id++);
        elem->set_node(0) = last_node;
        elem->set_node(1) = node;

        // Set neighbor links
        if (last_elem)
        {
          elem->set_neighbor(0, last_elem);
          last_elem->set_neighbor(1, elem);
        }

        last_elem = elem;
        last_node = node;
      }
    }
  }

  // If the mesh is replicated, everything that follows is unnecessary. Prepare and be done.
  if (_segment_mesh->is_replicated())
  {
    _segment_mesh->prepare_for_use();
    return;
  }

  // Find the Nodes on processor boundaries that we need to decide on owners for. Also set up
  // remote_elem links for neighbors we clearly don't have. We don't build the neighbor maps at
  // all, so all we have are nullptr and remote_elem. We can only do all of this after the mesh is
  // built because the mesh isn't necessarily built in the order Rays are traced
  std::vector<Node *> need_node_owners;
  for (const auto & trace_data : _study.getCachedTraces())
  {
    const auto num_segments = trace_data.numSegments();

    if (num_segments == 0)
      continue;

    dof_id_type start_node_id, start_elem_id;
    startingIDs(trace_data, start_node_id, start_elem_id);

    // Another part of the trace for this Ray happened before this one
    if ((_study.segmentsOnCacheTraces() ? trace_data._intersections
                                        : (unsigned long int)(trace_data._processor_crossings +
                                                              trace_data._trajectory_changes)) != 0)
    {
      // The element before start_elem (may be nullptr, meaning it didn't trace on this proc)
      Elem * previous_elem = _segment_mesh->query_elem_ptr(start_elem_id - 1);

      // We don't have the previous element segment, so it exists on another processor
      // Set the remote_elem and mark that we need to find out who owns the first node on this
      // trace
      if (!previous_elem)
      {
        Elem * start_elem = _segment_mesh->elem_ptr(start_elem_id);
        start_elem->set_neighbor(0, const_cast<RemoteElem *>(remote_elem));
        start_elem->node_ptr(0)->invalidate_processor_id();
        need_node_owners.push_back(start_elem->node_ptr(0));
      }
    }

    // If you have multiple sets of segments on a single processor, it is possible
    // to have part of the trace on another processor within the segments we know about.
    // In this case, we have to do some magic to the neighbor links so that the mesh
    // doesn't fail in assertions.
    if (!trace_data._last)
    {
      // The element after end_elem (may be nullptr, meaning it didn't happen on this proc)
      Elem * next_elem = _segment_mesh->query_elem_ptr(start_elem_id + num_segments);

      // We have the next element from another trace, set neighbors as we own both
      if (!next_elem)
      {
        Elem * end_elem = _segment_mesh->elem_ptr(start_elem_id + num_segments - 1);
        end_elem->set_neighbor(1, const_cast<RemoteElem *>(remote_elem));
        end_elem->node_ptr(1)->invalidate_processor_id();
        need_node_owners.push_back(end_elem->node_ptr(1));
      }
    }
  }

  // Sort through the neighboring bounding boxes and prepare requests to each processor that /may/
  // also have one of the nodes that we need to decide on an owner for
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> need_node_owners_sends;
  for (const Node * node : need_node_owners)
    for (const auto & neighbor_pid_bbox_pair : _inflated_neighbor_bboxes)
    {
      const auto neighbor_pid = neighbor_pid_bbox_pair.first;
      const auto & neighbor_bbox = neighbor_pid_bbox_pair.second;
      if (neighbor_bbox.contains_point(*node))
        need_node_owners_sends[neighbor_pid].push_back(node->id());
    }

  // Functor that takes in a set of incoming node IDs from a processor and decides on an owner.
  // For every node that we need an owner for, this should be called for said node on both
  // processors. Therefore, just pick the minimum processor ID as the owner. This should never
  // happen more than once for a single node because we're building 1D elems and we can only ever
  // have two procs that touch a node.
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> confirm_node_owners_sends;
  auto decide_node_owners_functor =
      [this, &confirm_node_owners_sends](processor_id_type pid,
                                         const std::vector<dof_id_type> & incoming_node_ids)
  {
    auto & confirm_pid = confirm_node_owners_sends[pid];
    for (const auto & node_id : incoming_node_ids)
    {
      Node * node = _segment_mesh->query_node_ptr(node_id);
      if (node)
      {
        mooseAssert(!node->valid_processor_id(), "Should be invalid");
        node->processor_id() = std::min(pid, processor_id());
        confirm_pid.push_back(node_id);
      }
    }
  };

  // Ship the nodes that need owners for and pick an owner when we receive
  Parallel::push_parallel_vector_data(
      _communicator, need_node_owners_sends, decide_node_owners_functor);

  // At this point, any nodes that we actually need ghosts for should have their processor_ids
  // satisfied. Anything that is left isn't actually ghosted and is ours. Therefore, set
  // everything left that is invalid to be owned by this proc.
  for (Node * node : need_node_owners)
    if (!node->valid_processor_id())
      node->processor_id() = processor_id();

  // We're done!
  _segment_mesh->prepare_for_use();
}

void
RayTracingMeshOutput::setupEquationSystem()
{
  TIME_SECTION("setupEquationSystem", 3, "Setting Up Ray Tracing MeshOutput Equation System");

  _es = std::make_unique<EquationSystems>(*_segment_mesh);
  _sys = &_es->add_system<libMesh::ExplicitSystem>("sys");
  _data_vars.clear();
  _aux_data_vars.clear();

  // Add variables for the basic properties if enabled
  _ray_id_var = invalid_uint;
  _intersections_var = invalid_uint;
  _pid_var = invalid_uint;
  _processor_crossings_var = invalid_uint;
  _trajectory_changes_var = invalid_uint;
  for (auto & prop : _pars.get<MultiMooseEnum>("output_properties"))
    switch (prop)
    {
      case 0: // ray_id (stationary and segments)
        _ray_id_var = _sys->add_variable("ray_id", CONSTANT, MONOMIAL);
        break;
      case 1: // intersections (segments only)
        if (_segmented_rays)
          _intersections_var = _sys->add_variable("intersections", CONSTANT, MONOMIAL);
        break;
      case 2: // pid (stationary and segments)
        _pid_var = _sys->add_variable("pid", CONSTANT, MONOMIAL);
        break;
      case 3: // processor_crossings (segments only)
        if (_segmented_rays)
          _processor_crossings_var = _sys->add_variable("processor_crossings", CONSTANT, MONOMIAL);
        break;
      case 4: // trajectory_changes (segments only)
        if (_segmented_rays)
          _trajectory_changes_var = _sys->add_variable("trajectory_changes", CONSTANT, MONOMIAL);
        break;
      default:
        mooseError("Invalid property");
    }

  const auto get_data_vars = [this](const bool aux)
  {
    // The data index -> variable result
    std::vector<std::pair<RayDataIndex, unsigned int>> vars;

    const auto from_names =
        aux ? (_output_aux_data ? &_study.rayAuxDataNames() : _output_aux_data_names)
            : (_output_data ? &_study.rayDataNames() : _output_data_names);

    // Nothing to output
    if (!from_names)
      return vars;

    const auto output_data_nodal = aux ? false : _output_data_nodal;

    for (const auto & name : *from_names)
    {
      const auto data_index =
          aux ? _study.getRayAuxDataIndex(name, true) : _study.getRayDataIndex(name, true);
      if (data_index == Ray::INVALID_RAY_DATA_INDEX)
      {
        const std::string names_param = aux ? "output_aux_data_names" : "output_data_names";
        const std::string data_prefix = aux ? "aux " : "";
        paramError(names_param, "The ray ", data_prefix, "data '", name, "' is not registered");
      }

      const std::string var_prefix = aux ? "aux_" : "";
      if (output_data_nodal)
        _sys->add_variable(var_prefix + name, FIRST, LAGRANGE);
      else
        _sys->add_variable(var_prefix + name, CONSTANT, MONOMIAL);
      const auto var_num = _sys->variable_number(var_prefix + name);
      vars.emplace_back(data_index, var_num);
    }

    return vars;
  };

  _data_vars = get_data_vars(false);
  _aux_data_vars = get_data_vars(true);

  // All done
  _es->init();
}

void
RayTracingMeshOutput::fillFields()
{
  TIME_SECTION("fillFields", 3, "Filling RayTracing MeshOutput Fields");

  // Helper for setting the solution (if enabled)
  const auto set_solution =
      [this](const DofObject * const dof, const unsigned int var, const Real value)
  {
    mooseAssert(dof, "Nullptr dof");
    if (var != invalid_uint)
    {
      const auto dof_number = dof->dof_number(_sys->number(), var, 0);
      _sys->solution->set(dof_number, value);
    }
  };

  // Helper for filling data and aux data (if enabled)
  const auto fill_data =
      [&set_solution](const DofObject * const dof, const auto & vars, const auto & data)
  {
    mooseAssert(dof, "Nullptr dof");
    for (const auto & [data_index, var_num] : vars)
      set_solution(dof, var_num, data[data_index]);
  };

  for (const auto & trace_data : _study.getCachedTraces())
  {
    auto intersection = trace_data._intersections;

    // No segments and not stationary; means this ray bounced off
    // this processor and never actually moved on this processor
    if (!trace_data.numSegments() && !trace_data.stationary())
      continue;

    dof_id_type node_id, elem_id;
    startingIDs(trace_data, node_id, elem_id);

    const Elem * elem = _segment_mesh->elem_ptr(elem_id);

    // Fill first node's nodal data if we need it; the loop that follows will handle
    // the rest of the nodes
    if (_output_data_nodal && _study.hasRayData() && !trace_data.stationary())
      fill_data(elem->node_ptr(0), _data_vars, trace_data._point_data[0]._data);

    const std::size_t start = trace_data.stationary() ? 0 : 1;
    for (const auto i : make_range(start, trace_data._point_data.size()))
    {
      mooseAssert(elem, "Nullptr elem");

      // Elemental ID and pid
      set_solution(elem, _ray_id_var, trace_data._ray_id);
      set_solution(elem, _pid_var, processor_id());
      // Elemental properties that only apply to segments
      if (!trace_data.stationary())
      {
        set_solution(elem, _intersections_var, intersection++);
        set_solution(elem, _processor_crossings_var, trace_data._processor_crossings);
        set_solution(elem, _trajectory_changes_var, trace_data._trajectory_changes);
      }
      const auto & point_data = trace_data._point_data[i];
      // Data fields
      if (_output_data_nodal && !trace_data.stationary())
        fill_data(elem->node_ptr(1), _data_vars, point_data._data);
      else
        fill_data(elem, _data_vars, point_data._data);
      // Aux data fields
      fill_data(elem, _aux_data_vars, point_data._aux_data);

      // Advance to the next element
      if (!trace_data.stationary())
        elem = elem->neighbor_ptr(1);
    }
  }

  _sys->solution->close();
  _sys->update();
}

void
RayTracingMeshOutput::buildBoundingBoxes()
{
  TIME_SECTION("buildBoundingBoxes", 3, "Building Bounding Boxes for RayTracing Mesh Output");

  // Not used in the one proc case
  if (_communicator.size() == 1)
    return;

  // Local bounding box
  _bbox = MeshTools::create_local_bounding_box(_mesh_ptr->getMesh());

  // Gather the bounding boxes of all processors
  std::vector<std::pair<Point, Point>> bb_points = {static_cast<std::pair<Point, Point>>(_bbox)};
  _communicator.allgather(bb_points, true);
  _inflated_bboxes.resize(_communicator.size());
  for (processor_id_type pid = 0; pid < _communicator.size(); ++pid)
  {
    BoundingBox pid_bbox = static_cast<BoundingBox>(bb_points[pid]);
    pid_bbox.scale(0.01);
    _inflated_bboxes[pid] = pid_bbox;
  }

  // Find intersecting (neighbor) bounding boxes
  _inflated_neighbor_bboxes.clear();
  for (processor_id_type pid = 0; pid < _communicator.size(); ++pid)
    if (pid != processor_id())
    {
      // Insert if the searched processor's bbox intersects my bbox
      const auto & pid_bbox = _inflated_bboxes[pid];
      if (_bbox.intersects(pid_bbox))
        _inflated_neighbor_bboxes.emplace_back(pid, pid_bbox);
    }
}

void
RayTracingMeshOutput::startingIDs(const TraceData & trace_data,
                                  dof_id_type & start_node_id,
                                  dof_id_type & start_elem_id) const
{
  const auto [begin_node_id, begin_elem_id] = _ray_starting_id_map.at(trace_data._ray_id);

  const auto offset =
      trace_data.stationary()
          ? 1
          : (_study.segmentsOnCacheTraces()
                 ? trace_data._intersections
                 : (trace_data._processor_crossings + trace_data._trajectory_changes));

  start_node_id = begin_node_id + offset;
  start_elem_id = begin_elem_id + offset;
}

dof_id_type
RayTracingMeshOutput::neededNodes(const TraceData & trace_data) const
{
  if (_study.segmentsOnCacheTraces())
    return trace_data._intersections + trace_data._point_data.size();
  return trace_data._processor_crossings + trace_data._trajectory_changes +
         trace_data._point_data.size();
}
