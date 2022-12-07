//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"

#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

namespace GeneralFieldTransfer
{
Number BetterOutOfMeshValue = std::numeric_limits<Real>::infinity();
}

InputParameters
MultiAppGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  // Expansion default to make a node in the target mesh overlapping with a node in the origin
  // mesh always register as being inside the origin application bounding box. The contains_point
  // bounding box checks uses exact comparisons
  params.addRangeCheckedParam<Real>("bbox_factor",
                                    1.00000001,
                                    "bbox_factor>0",
                                    "Factor to inflate or deflate the source app bounding boxes");
  params.addRangeCheckedParam<std::vector<Real>>(
      "fixed_bounding_box_size",
      "fixed_bounding_box_size >= 0",
      "Override source app bounding box size(s) for searches. App bounding boxes will be grown "
      "symmetrically. Only non-zero components passed will override.");

  // Block restrictions
  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "Subdomain restriction to transfer from (defaults to all the origin app domain)");
  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "Subdomain restriction to transfer to, (defaults to all the target app domain)");

  // Boundary restrictions
  params.addParam<std::vector<BoundaryName>>(
      "from_boundaries",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "to_boundaries",
      "The boundary we are transferring to (if not specified, whole domain is used).");
  MooseEnum nodes_or_sides("nodes sides", "sides");
  params.addParam<MooseEnum>("elemental_boundary_restriction",
                             nodes_or_sides,
                             "Whether elemental variable boundary restriction is considered by "
                             "element side or element nodes");

  // Search options
  params.addParam<bool>(
      "greedy_search",
      false,
      "Whether or not to send a point to all the domains. If true, all the processors will be "
      "checked for a given point."
      "The code will be slow if this flag is on but it will give a better solution.");
  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");

  params.addParamNamesToGroup(
      "to_blocks from_blocks to_boundaries from_boundaries elemental_boundary_restriction",
      "Transfer spatial restriction");
  params.addParamNamesToGroup("greedy_search error_on_miss", "Search algorithm");
  params.addParamNamesToGroup("bbox_factor fixed_bounding_box_size", "Source app bounding box");
  return params;
}

MultiAppGeneralFieldTransfer::MultiAppGeneralFieldTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _elemental_boundary_restriction_on_sides(
        getParam<MooseEnum>("elemental_boundary_restriction") == "sides"),
    _greedy_search(getParam<bool>("greedy_search")),
    _num_overlaps(0),
    _error_on_miss(getParam<bool>("error_on_miss")),
    _bbox_factor(getParam<Real>("bbox_factor")),
    _fixed_bbox_size(isParamValid("fixed_bounding_box_size")
                         ? getParam<std::vector<Real>>("fixed_bounding_box_size")
                         : std::vector<Real>(3, 0))
{
  if (_to_var_names.size() == _from_var_names.size())
    _var_size = _to_var_names.size();
  else
    paramError("variable", "The number of variables to transfer to and from should be equal");
}

void
MultiAppGeneralFieldTransfer::initialSetup()
{
  MultiAppConservativeTransfer::initialSetup();

  // Use IDs for block and boundary restriction
  // Loop over all source problems
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    const auto & from_moose_mesh = _from_problems[i_from]->mesh(_displaced_source_mesh);
    if (isParamValid("from_blocks"))
    {
      auto & blocks = getParam<std::vector<SubdomainName>>("from_blocks");
      std::vector<SubdomainID> ids = from_moose_mesh.getSubdomainIDs(blocks);
      _from_blocks.insert(ids.begin(), ids.end());
      if (_from_blocks.size() != blocks.size())
        paramError("from_blocks", "Some blocks were not found in the mesh");
    }

    if (isParamValid("from_boundaries"))
    {
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundaries");
      std::vector<BoundaryID> boundary_ids = from_moose_mesh.getBoundaryIDs(boundary_names);
      _from_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
      if (_from_boundaries.size() != boundary_names.size())
        paramError("from_boundaries", "Some boundaries were not found in the mesh");
    }
  }

  // Loop over all target problems
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    const auto & to_moose_mesh = _to_problems[i_to]->mesh(_displaced_target_mesh);
    if (isParamValid("to_blocks"))
    {
      auto & blocks = getParam<std::vector<SubdomainName>>("to_blocks");
      std::vector<SubdomainID> ids = to_moose_mesh.getSubdomainIDs(blocks);
      _to_blocks.insert(ids.begin(), ids.end());
      if (_to_blocks.size() != blocks.size())
        paramError("to_blocks", "Some blocks were not found in the mesh");
    }

    if (isParamValid("to_boundaries"))
    {
      auto & boundary_names = getParam<std::vector<BoundaryName>>("to_boundaries");
      std::vector<BoundaryID> boundary_ids = to_moose_mesh.getBoundaryIDs(boundary_names);
      _to_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
      if (_to_boundaries.size() != boundary_names.size())
        paramError("to_boundaries", "Some boundaries were not found in the mesh");
    }
  }

  // Create the point locators to locate evaluation points in the origin mesh(es)
  _from_point_locators.resize(_from_problems.size());
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    const auto & from_moose_mesh = _from_problems[i_from]->mesh(_displaced_source_mesh);
    _from_point_locators[i_from] =
        PointLocatorBase::build(TREE_LOCAL_ELEMENTS, from_moose_mesh.getMesh());
    _from_point_locators[i_from]->enable_out_of_mesh_mode();
  }
}

void
MultiAppGeneralFieldTransfer::execute()
{
  getAppInfo();

  // Reset number of overlaps found
  _num_overlaps = 0;

  // loop over the vector of variables and make the transfer one by one
  for (unsigned int i = 0; i < _var_size; ++i)
    transferVariable(i);

  // Warn user about overlaps
  if (_num_overlaps)
    mooseWarning(
        "Multiple origin positions & values were found. "
        "Over all target points and variables: " +
        std::to_string(_num_overlaps) +
        " instances.\nAre multiple subapps overlapping?\n"
        "Are some target mesh locations exactly equidistant from nodes in origin mesh(es)?");

  postExecute();
}

void
MultiAppGeneralFieldTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

  // Get the bounding boxes for the "from" domains.
  // Clean up _bboxes
  _bboxes.clear();
  _bboxes = getRestrictedFromBoundingBoxes();

  // Expand bounding boxes. Some desired points might be excluded
  // without an expansion
  for (auto & box : _bboxes)
  {
    // libmesh set an invalid bounding box using this code
    // for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    // {
    //   this->first(i)  =  std::numeric_limits<Real>::max();
    //   this->second(i) = -std::numeric_limits<Real>::max();
    // }
    // If it is an invalid box, we should skip it
    if (box.first(0) == std::numeric_limits<Real>::max())
      continue;

    auto width = box.second - box.first;
    box.second += width * (_bbox_factor - 1);
    box.first -= width * (_bbox_factor - 1);
  }

  // Figure out how many "from" domains each processor owns.
  _froms_per_proc.clear();
  _froms_per_proc = getFromsPerProc();

  // Find outgoing target points
  // We need to know what points we need to send which processors
  // One processor will receive many points from many processors
  // One point may go to different processors
  ProcessorToPointVec outgoing_points;
  extractOutgoingPoints(_to_var_names[i], outgoing_points);

  prepareEvaluationOfInterpValues(_from_var_names[i]);

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor = [this](processor_id_type /*pid*/,
                               const std::vector<Point> & incoming_points,
                               std::vector<std::pair<Real, Real>> & outgoing_vals)
  {
    outgoing_vals.resize(
        incoming_points.size(),
        {GeneralFieldTransfer::BetterOutOfMeshValue, GeneralFieldTransfer::BetterOutOfMeshValue});
    // Evaluate interpolation values for these incoming points
    evaluateInterpValues(incoming_points, outgoing_vals);
  };

  DofobjectToInterpValVec dofobject_to_valsvec(_to_problems.size());
  InterpCaches interp_caches(_to_problems.size());

  // Copy data out to incoming_vals_ids
  auto action_functor = [this, &i, &dofobject_to_valsvec, &interp_caches](
                            processor_id_type pid,
                            const std::vector<Point> & my_outgoing_points,
                            const std::vector<std::pair<Real, Real>> & incoming_vals)
  {
    auto & pointInfoVec = _processor_to_pointInfoVec[pid];

    // Cache interpolation values for each dof object
    cacheIncomingInterpVals(pid,
                            _to_var_names[i],
                            pointInfoVec,
                            my_outgoing_points,
                            incoming_vals,
                            dofobject_to_valsvec,
                            interp_caches);
  };

  // We assume incoming_vals_ids is ordered in the same way as outgoing_points
  // Hopefully, pull_parallel_vector_data will not mess up this
  const std::pair<Real, Real> * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, ex);

  // Set cached values into solution vector
  setSolutionVectorValues(_to_var_names[i], dofobject_to_valsvec, interp_caches);
}

void
MultiAppGeneralFieldTransfer::locatePointReceivers(const Point point,
                                                   std::set<processor_id_type> & processors)
{
  // Check which processors include this point
  // One point might have more than one points
  bool found = false;
  unsigned int from0 = 0;
  // Find which bboxes might have the nearest node to this point.
  Real nearest_max_distance = std::numeric_limits<Real>::max();
  for (const auto & bbox : _bboxes)
  {
    Real distance = bboxMaxDistance(point, bbox);
    if (distance < nearest_max_distance)
      nearest_max_distance = distance;
  }
  for (processor_id_type i_proc = 0; i_proc < n_processors();
       from0 += _froms_per_proc[i_proc], ++i_proc)
    for (unsigned int i_from = from0; i_from < from0 + _froms_per_proc[i_proc]; ++i_from)
    {
      Real distance = bboxMinDistance(point, _bboxes[i_from]);
      // We will not break here because we want to send a point to all possible source domains
      if (_greedy_search || distance <= nearest_max_distance ||
          _bboxes[i_from].contains_point(point))
      {
        processors.insert(i_proc);
        found = true;
      }
    }
  // Error out if we could not find this point when ask us to do so
  if (!found && _error_on_miss)
    mooseError("Cannot locate point ", point, " \n ", "mismatched meshes are used");
}

void
MultiAppGeneralFieldTransfer::cacheOutgoingPointInfor(const Point point,
                                                      const dof_id_type dof_object_id,
                                                      const unsigned int problem_id,
                                                      ProcessorToPointVec & outgoing_points)
{
  std::set<processor_id_type> processors;
  // Try to find which processors
  processors.clear();
  locatePointReceivers(point, processors);

  // We need to send these data to these processors
  for (auto pid : processors)
  {
    outgoing_points[pid].push_back(point);
    // Store point information
    // We can use these information when insert values to solution vector
    PointInfor pointinfo;
    pointinfo.problem_id = problem_id;
    pointinfo.dof_object_id = dof_object_id;
    pointinfo.offset = 0;
    _processor_to_pointInfoVec[pid].push_back(pointinfo);
  }
}

void
MultiAppGeneralFieldTransfer::extractOutgoingPoints(const VariableName & var_name,
                                                    ProcessorToPointVec & outgoing_points)
{
  // Clean up the map from processor to pointInfo vector
  // This map should be consistent with outgoing_points
  _processor_to_pointInfoVec.clear();

  // Loop over all problems
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    // libMesh EquationSystems
    // NOTE: we would expect to set variables from the displaced equation system here
    auto & es = getEquationSystem(*_to_problems[i_to], false);
    // libMesh system that has this variable
    // Assume var name is unique in an equation system
    System * to_sys = find_sys(es, var_name);
    auto sys_num = to_sys->number();
    auto var_num = to_sys->variable_number(var_name);
    // FEM type info
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    // Moose mesh
    const auto & to_moose_mesh = _to_problems[i_to]->mesh(_displaced_target_mesh);
    const auto & to_mesh = to_moose_mesh.getMesh();

    // We support more general variables via libMesh GenericProjector
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      GeneralFieldTransfer::RecordRequests<Number> f;
      GeneralFieldTransfer::RecordRequests<Gradient> g;
      GeneralFieldTransfer::NullAction<Number> nullsetter;
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<GeneralFieldTransfer::RecordRequests<Number>,
                                GeneralFieldTransfer::RecordRequests<Gradient>,
                                Number,
                                GeneralFieldTransfer::NullAction<Number>>
          request_gather(*to_sys, f, &g, nullsetter, varvec);

      const auto & to_begin = _to_blocks.empty()
                                  ? to_mesh.active_local_elements_begin()
                                  : to_mesh.active_local_subdomain_set_elements_begin(_to_blocks);

      const auto & to_end = _to_blocks.empty()
                                ? to_mesh.active_local_elements_end()
                                : to_mesh.active_local_subdomain_set_elements_end(_to_blocks);

      ConstElemRange to_elem_range(to_begin, to_end);

      request_gather.project(to_elem_range);

      for (Point p : f.points_requested())
      {
        // using dof_object_id 0 for value requests
        this->cacheOutgoingPointInfor(p, 0, i_to, outgoing_points);
      }

      // This is going to require more complicated transfer work
      if (!g.points_requested().empty())
      {
        mooseError("We don't currently support variables with gradient degrees of freedom");
      }
    }
    else if (is_nodal)
    {
      for (const auto & node : to_mesh.local_node_ptr_range())
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted transfer and current node does not have
        // specified blocks
        if (!_to_blocks.empty() && !hasBlocks(_to_blocks, to_moose_mesh, node))
          continue;

        if (!_to_boundaries.empty() && !hasBoundaries(_to_boundaries, to_moose_mesh, node))
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfor(*node + _to_positions[i_to], node->id(), i_to, outgoing_points);
      }
    }
    else // Elemental
    {
      for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted block and current elem does not have
        // specified blocks
        if (!_to_blocks.empty() && !hasBlocks(_to_blocks, elem))
          continue;

        if (!_to_boundaries.empty() && !hasBoundaries(_to_boundaries, to_moose_mesh, elem))
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfor(
            elem->vertex_average() + _to_positions[i_to], elem->id(), i_to, outgoing_points);
      } // for
    }   // else
  }     // for
}

void
MultiAppGeneralFieldTransfer::extractLocalFromBoundingBoxes(std::vector<BoundingBox> & local_bboxes)
{
  local_bboxes.resize(_froms_per_proc[processor_id()]);
  // Find the index to the first of this processor's local bounding boxes.
  unsigned int local_start = 0;
  for (processor_id_type i_proc = 0; i_proc < n_processors() && i_proc != processor_id(); ++i_proc)
    local_start += _froms_per_proc[i_proc];

  // Extract the local bounding boxes.
  for (unsigned int i_from = 0; i_from < _froms_per_proc[processor_id()]; ++i_from)
    local_bboxes[i_from] = _bboxes[local_start + i_from];
}

void
MultiAppGeneralFieldTransfer::cacheIncomingInterpVals(
    processor_id_type pid,
    const VariableName & var_name,
    std::vector<PointInfor> & pointInfoVec,
    const std::vector<Point> & point_requests,
    const std::vector<std::pair<Real, Real>> & incoming_vals,
    DofobjectToInterpValVec & dofobject_to_valsvec,
    InterpCaches & interp_caches)
{
  mooseAssert(pointInfoVec.size() == incoming_vals.size(),
              "Number of dof objects does not equal to the number of incoming values");

  dof_id_type val_offset = 0;
  for (auto & pointinfo : pointInfoVec)
  {
    const auto problem_id = pointinfo.problem_id;
    const auto dof_object_id = pointinfo.dof_object_id;

    const std::pair<unsigned int, dof_id_type> dofobject(problem_id, dof_object_id);

    // libMesh EquationSystems
    // NOTE: we would expect to set variables from the displaced equation system here
    auto & es = getEquationSystem(*_to_problems[problem_id], false);
    // libMesh system
    System * to_sys = find_sys(es, var_name);

    auto var_num = to_sys->variable_number(var_name);
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    if (fe_type.order > CONSTANT && !is_nodal)
    {
      // Defining only boundary values will not be enough to describe the variable, disallow it
      if (_to_boundaries.size() && fe_type.order > 0)
        mooseError("Higher order elemental variables are not supported for target-boundary "
                   "restricted transfers");

      InterpCache & cache = interp_caches[problem_id];
      Point p = point_requests[val_offset];
      const Number val = incoming_vals[val_offset].first;
      // We should only have one value for each variable at any given point.
      if (cache.count(p) != 0 && !GeneralFieldTransfer::isBetterOutOfMeshValue(val) &&
          !MooseUtils::absoluteFuzzyEqual(cache[p], val))
        _num_overlaps++;
      if (!GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        cache[p] = val;
    }
    else
    {
      // Use this dof object pointer, so we can handle
      // both element and node using the same code
#ifdef NDEBUG
      DofObject * dof_object_ptr = nullptr;
      // It is a node
      if (is_nodal)
        dof_object_ptr = to_mesh.node_ptr(dof_object_id);
      // It is an element
      else
        dof_object_ptr = to_mesh.elem_ptr(dof_object_id);

      // We should only be supporting nodal and constant elemental
      // variables in this code path; if we see multiple DoFs on one
      // object we should have been using GenericProjector
      mooseAssert(dof_object_ptr->n_dofs(sys_num, var_num) == 1,
                  "Unexpectedly found " << dof_object_ptr->n_dofs(sys_num, var_num)
                                        << "dofs instead of 1");
#endif

      auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

      // Check if we visited this dof object earlier
      auto values_ptr = dofobject_to_val.find(dof_object_id);
      // We did not visit this
      if (values_ptr == dofobject_to_val.end())
      {
        // Values for this dof object
        auto & val = dofobject_to_val[dof_object_id];
        // Interpolation value
        val.interp = incoming_vals[val_offset].first;
        // Where this value came from
        val.pid = pid;
        // Distance
        val.distance = incoming_vals[val_offset].second;
      }
      else
      {
        auto & val = values_ptr->second;
        // We adopt values that are, in order of priority
        // - valid
        // - closest distance
        // - the smallest rank with the same distance
        if (!GeneralFieldTransfer::isBetterOutOfMeshValue(incoming_vals[val_offset].first) &&
            (MooseUtils::absoluteFuzzyGreaterThan(val.distance, incoming_vals[val_offset].second) ||
             ((val.pid > pid || _greedy_search) &&
              MooseUtils::absoluteFuzzyEqual(val.distance, incoming_vals[val_offset].second))))
        {
          // Count disagreeing overlaps
          if (MooseUtils::absoluteFuzzyEqual(val.distance, incoming_vals[val_offset].second) &&
              !MooseUtils::absoluteFuzzyEqual(val.interp, incoming_vals[val_offset].first))
            _num_overlaps++;
          val.interp = incoming_vals[val_offset].first;
          val.pid = pid;
          val.distance = incoming_vals[val_offset].second;
        }
      }
    }

    // Move it to next position
    val_offset++;
  }
}

void
MultiAppGeneralFieldTransfer::setSolutionVectorValues(
    const VariableName & var_name,
    const DofobjectToInterpValVec & dofobject_to_valsvec,
    const InterpCaches & interp_caches)
{
  for (unsigned int problem_id = 0; problem_id < _to_problems.size(); ++problem_id)
  {
    auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

    // libMesh EquationSystems
    // NOTE: we would expect to set variables from the displaced equation system here
    auto & es = getEquationSystem(*_to_problems[problem_id], false);

    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    const MeshBase & to_mesh = _to_problems[problem_id]->mesh(_displaced_target_mesh).getMesh();
    auto var_num = to_sys->variable_number(var_name);
    auto sys_num = to_sys->number();

    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    if (fe_type.order > CONSTANT && !is_nodal)
    {
      // We may need to use existing data values in places where the
      // from app domain doesn't overlap
      MeshFunction to_func(es, *to_sys->current_local_solution, to_sys->get_dof_map(), var_num);
      to_func.init();

      GeneralFieldTransfer::CachedData<Number> f(interp_caches[problem_id], to_func);
      libMesh::VectorSetAction<Number> setter(*to_sys->solution);
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<GeneralFieldTransfer::CachedData<Number>,
                                GeneralFieldTransfer::CachedData<Gradient>,
                                Number,
                                libMesh::VectorSetAction<Number>>
          set_solution(*to_sys, f, nullptr, setter, varvec);

      ConstElemRange active_local_elem_range(to_mesh.active_local_elements_begin(),
                                             to_mesh.active_local_elements_end());

      set_solution.project(active_local_elem_range);
    }
    else
    {
      for (auto & val_pair : dofobject_to_val)
      {
        auto dof_object_id = val_pair.first;

        const DofObject * dof_object = nullptr;
        if (is_nodal)
          dof_object = to_mesh.node_ptr(dof_object_id);
        else
          dof_object = to_mesh.elem_ptr(dof_object_id);

        auto dof = dof_object->dof_number(sys_num, var_num, 0);

        auto val = val_pair.second.interp;

        // This will happen if meshes are mismatched
        if (_error_on_miss && GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        {
          if (is_nodal)
            mooseError("Node ", dof_object_id, " for app ", problem_id, " could not be located ");
          else
            mooseError(
                "Element ", dof_object_id, " for app ", problem_id, " could not be located ");
        }

        // We should not put garbage into solution vector
        if (GeneralFieldTransfer::isBetterOutOfMeshValue(val))
          continue;

        to_sys->solution->set(dof, val);
      }
    }

    to_sys->solution->close();
    // Sync local solutions
    to_sys->update();
  }
}

bool
MultiAppGeneralFieldTransfer::hasBlocks(std::set<SubdomainID> & blocks, const Elem * elem) const
{
  return blocks.find(elem->subdomain_id()) != blocks.end();
}

bool
MultiAppGeneralFieldTransfer::hasBlocks(std::set<SubdomainID> & blocks,
                                        const MooseMesh & mesh,
                                        const Node * node) const
{
  const std::set<SubdomainID> & node_blocks = mesh.getNodeBlockIds(*node);
  std::set<SubdomainID> u;
  std::set_intersection(blocks.begin(),
                        blocks.end(),
                        node_blocks.begin(),
                        node_blocks.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppGeneralFieldTransfer::hasBlocks(std::set<SubdomainID> & blocks,
                                        unsigned int i_from,
                                        const Point & point) const
{
  const Elem * elem = (*_from_point_locators[i_from])(point - _from_positions[i_from], &blocks);
  return (elem != nullptr);
}

bool
MultiAppGeneralFieldTransfer::hasBoundaries(std::set<BoundaryID> & boundaries,
                                            const MooseMesh & mesh,
                                            const Node * node) const
{
  const BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> vec_to_fill;
  bnd_info.boundary_ids(node, vec_to_fill);
  std::set<BoundaryID> vec_to_fill_set(vec_to_fill.begin(), vec_to_fill.end());
  std::set<BoundaryID> u;
  std::set_intersection(boundaries.begin(),
                        boundaries.end(),
                        vec_to_fill_set.begin(),
                        vec_to_fill_set.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppGeneralFieldTransfer::hasBoundaries(std::set<BoundaryID> & boundaries,
                                            const MooseMesh & mesh,
                                            const Elem * elem) const
{
  // Get all boundaries each side of the element is part of
  const BoundaryInfo & bnd_info = mesh.getMesh().get_boundary_info();
  std::vector<BoundaryID> vec_to_fill;
  std::vector<BoundaryID> vec_to_fill_temp;
  if (_elemental_boundary_restriction_on_sides)
    for (auto side : make_range(elem->n_sides()))
    {
      bnd_info.boundary_ids(elem, side, vec_to_fill_temp);
      vec_to_fill.insert(vec_to_fill.end(), vec_to_fill_temp.begin(), vec_to_fill_temp.end());
    }
  else
    for (auto node_index : make_range(elem->n_nodes()))
    {
      bnd_info.boundary_ids(elem->node_ptr(node_index), vec_to_fill_temp);
      vec_to_fill.insert(vec_to_fill.end(), vec_to_fill_temp.begin(), vec_to_fill_temp.end());
    }
  std::set<BoundaryID> vec_to_fill_set(vec_to_fill.begin(), vec_to_fill.end());

  // Look for a match between the boundaries from the restriction and those near the element
  std::set<BoundaryID> u;
  std::set_intersection(boundaries.begin(),
                        boundaries.end(),
                        vec_to_fill_set.begin(),
                        vec_to_fill_set.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

Real
MultiAppGeneralFieldTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox)
{
  std::array<Point, 2> source_points = {{bbox.first, bbox.second}};

  std::array<Point, 8> all_points;
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2 * y + 4 * z] =
            Point(source_points[x](0), source_points[y](1), source_points[z](2));

  Real max_distance = 0.;

  for (unsigned int i = 0; i < 8; i++)
  {
    Real distance = (p - all_points[i]).norm();
    if (distance > max_distance)
      max_distance = distance;
  }

  return max_distance;
}

Real
MultiAppGeneralFieldTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox)
{
  std::array<Point, 2> source_points = {{bbox.first, bbox.second}};

  std::array<Point, 8> all_points;
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2 * y + 4 * z] =
            Point(source_points[x](0), source_points[y](1), source_points[z](2));

  Real min_distance = std::numeric_limits<Real>::max();

  for (unsigned int i = 0; i < 8; i++)
  {
    Real distance = (p - all_points[i]).norm();
    if (distance < min_distance)
      min_distance = distance;
  }

  return min_distance;
}

std::vector<BoundingBox>
MultiAppGeneralFieldTransfer::getRestrictedFromBoundingBoxes()
{
  std::vector<std::pair<Point, Point>> bb_points(_from_meshes.size());
  const Real min_r = std::numeric_limits<Real>::lowest();
  const Real max_r = std::numeric_limits<Real>::max();

  for (const auto j : make_range(_from_meshes.size()))
  {

    Point min(max_r, max_r, max_r);
    Point max(min_r, min_r, min_r);
    bool at_least_one = false;
    const auto & from_mesh = _from_problems[j]->mesh(_displaced_source_mesh);

    for (auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                from_mesh.getMesh().local_elements_end()))
    {
      if (!_from_blocks.empty() && !hasBlocks(_from_blocks, elem))
        continue;

      for (auto & node : elem->node_ref_range())
      {
        if (!_from_boundaries.empty() && !hasBoundaries(_from_boundaries, from_mesh, &node))
          continue;

        at_least_one = true;
        for (const auto i : make_range(LIBMESH_DIM))
        {
          min(i) = std::min(min(i), node(i));
          max(i) = std::max(max(i), node(i));
        }
      }
    }
    BoundingBox bbox(min, max);
    if (!at_least_one)
      bbox.min() = max; // If we didn't hit any nodes, this will be _the_ minimum bbox
    else
    {
      // Translate the bounding box to the from domain's position.
      bbox.first += _from_positions[j];
      bbox.second += _from_positions[j];
    }

    // Cast the bounding box into a pair of points (so it can be put through
    // MPI communication).
    bb_points[j] = static_cast<std::pair<Point, Point>>(bbox);
  }

  // Serialize the bounding box points.
  _communicator.allgather(bb_points);

  // Recast the points back into bounding boxes and return.
  std::vector<BoundingBox> bboxes(bb_points.size());
  for (const auto i : make_range(bb_points.size()))
    bboxes[i] = static_cast<BoundingBox>(bb_points[i]);

  // Check for a user-set fixed bounding box size and modify the sizes as appropriate
  if (_fixed_bbox_size != std::vector<Real>(3, 0))
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      if (!MooseUtils::absoluteFuzzyEqual(_fixed_bbox_size[i], 0))
        for (const auto j : make_range(bboxes.size()))
        {
          const auto current_width = (bboxes[j].second - bboxes[j].first)(i);
          bboxes[j].first(i) -= (_fixed_bbox_size[i] - current_width) / 2;
          bboxes[j].second(i) += (_fixed_bbox_size[i] - current_width) / 2;
        }

  return bboxes;
}
