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
  params.addParam<Real>("bbox_tol", 0.1, "How much want to relax bounding boxes");

  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "The blocks we are transferring to (if not specified, whole domain is used).");

  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "The blocks we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "from_boundaries",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "to_boundaries",
      "The boundary we are transferring to (if not specified, whole domain is used).");
  params.addParam<bool>(
      "greedy_search",
      false,
      "Whether or not to send a point to all the domains. If true, all the processors will be "
      "checked for a given point."
      "The code will be slow if this flag is on but it will give a better solution.");
  params.addParam<unsigned int>("num_nearest_points",
                                1,
                                "Number of nearest source (from) points will be chosen to "
                                "construct a value for the target point.");
  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");
  params.addParamNamesToGroup("to_blocks from_blocks to_boundaries from_boundaries",
                              "Transfer spatial restriction");
  return params;
}

MultiAppGeneralFieldTransfer::MultiAppGeneralFieldTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _error_on_miss(getParam<bool>("error_on_miss")),
    _bbox_tol(getParam<Real>("bbox_tol")),
    _greedy_search(getParam<bool>("greedy_search")),
    _num_nearest_points(getParam<unsigned int>("num_nearest_points"))
{
  if (_to_var_names.size() == _from_var_names.size())
    _var_size = _to_var_names.size();
  else
    paramError("variable", "The number of variables to transfer to and from should be equal");
}

void
MultiAppGeneralFieldTransfer::execute()
{
  _console << "Beginning GeneralFieldTransfer " << name() << std::endl;

  getAppInfo();

  // loop over the vector of variables and make the transfer one by one
  for (unsigned int i = 0; i < _var_size; ++i)
    transferVariable(i);

  _console << "Finished GeneralFieldTransfer " << name() << std::endl;

  postExecute();
}

void
MultiAppGeneralFieldTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

  // Get the bounding boxes for the "from" domains.
  // Clean up _bboxes
  _bboxes.clear();
  //_bboxes = getFromBoundingBoxes();
  _bboxes = getRestrictedFromBoundingBoxes();

  // Expand bounding boxes. Some right points might be excluded
  // without an expansion
  for (auto & box : _bboxes)
  {
    // libmesh set an invalid bounding box using this code
    // for (unsigned int i=0; i<LIBMESH_DIM; i++)
    // {
    //   this->first(i)  =  std::numeric_limits<Real>::max();
    //   this->second(i) = -std::numeric_limits<Real>::max();
    // }
    // If it is an invalid box, we should skip it
    if (box.first(0) == std::numeric_limits<Real>::max())
      continue;

    auto width = box.second - box.first;
    box.second += width * _bbox_tol;
    box.first -= width * _bbox_tol;
  }

  // Figure out how many "from" domains each processor owns.
  // Clean up _froms_per_proc
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
    auto & es = _to_problems[i_to]->es();
    // libMesh system that has this variable
    // Assume var name is unique in an equation system
    System * to_sys = find_sys(es, var_name);
    auto sys_num = to_sys->number();
    auto var_num = to_sys->variable_number(var_name);
    // libMesh MeshBase
    auto & to_mesh = to_sys->get_mesh();
    auto & fe_type = to_sys->variable_type(var_num);
    // FEM type info
    bool is_nodal = fe_type.family == LAGRANGE;

    // Moose mesh
    MooseMesh * to_moose_mesh = &_to_problems[i_to]->mesh();

    std::set<SubdomainID> _to_blocks;

    // Take users' input block names
    // Change them to ids
    // Store then in a member variables
    if (isParamValid("to_blocks"))
    {
      // User input block names
      auto & blocks = getParam<std::vector<SubdomainName>>("to_blocks");
      // Subdomain ids
      std::vector<SubdomainID> ids = to_moose_mesh->getSubdomainIDs(blocks);
      // Store these ids
      _to_blocks.insert(ids.begin(), ids.end());
    }

    std::set<BoundaryID> _to_boundaries;
    if (isParamValid("to_boundaries"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("to_boundaries");
      std::vector<BoundaryID> boundary_ids = to_moose_mesh->getBoundaryIDs(boundary_names);
      // Store these ids
      _to_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
    }

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

      const MeshBase::element_iterator to_begin =
          _to_blocks.empty() ? to_mesh.active_local_elements_begin()
                             : to_mesh.active_local_subdomain_set_elements_begin(_to_blocks);

      const MeshBase::element_iterator to_end =
          _to_blocks.empty() ? to_mesh.active_local_elements_end()
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
        if (!_to_blocks.empty() && !hasBlocks(_to_blocks, *to_moose_mesh, node))
          continue;

        if (!_to_boundaries.empty() && !hasBoundaries(_to_boundaries, *to_moose_mesh, node))
          continue;

        // Cache point information
        // We will use this information later for setting values back to solution vectors
        cacheOutgoingPointInfor(*node + _to_positions[i_to], node->id(), i_to, outgoing_points);
      }
    }
    else // Elemental
    {
      if (!_to_boundaries.empty())
      {
        mooseError("You can not restrict an elemental variable to any boundary");
      }
      for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted block and current elem does not have
        // specified blocks
        if (!_to_blocks.empty() && !hasBlocks(_to_blocks, elem))
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
  {
    local_start += _froms_per_proc[i_proc];
  }

  // Extract the local bounding boxes.
  for (unsigned int i_from = 0; i_from < _froms_per_proc[processor_id()]; ++i_from)
  {
    local_bboxes[i_from] = _bboxes[local_start + i_from];
  }
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
              " Number of dof objects does not equal to the number of incoming values");

  dof_id_type val_offset = 0;
  for (auto & pointinfo : pointInfoVec)
  {
    const auto problem_id = pointinfo.problem_id;
    const auto dof_object_id = pointinfo.dof_object_id;

    const std::pair<unsigned int, dof_id_type> dofobject(problem_id, dof_object_id);

    // libMesh EquationSystems
    auto & es = _to_problems[problem_id]->es();
    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    // MeshBase & to_mesh = _to_meshes[problem_id]->getMesh();
    auto var_num = to_sys->variable_number(var_name);
    // auto sys_num = to_sys->number();
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    if (fe_type.order > CONSTANT && !is_nodal)
    {
      InterpCache & cache = interp_caches[problem_id];
      Point p = point_requests[val_offset];
      // We should only have one value for each variable at any given point.
      libmesh_assert(cache.count(p) == 0);
      const Number val = incoming_vals[val_offset].first;
      if (!GeneralFieldTransfer::isBetterOutOfMeshValue(val))
        cache[p] = val;
    }
    else
    {
      // Use this dof object pointer, so we can handle
      // both element and node using the same code
      // DofObject * dof_object_ptr = nullptr;
      // It is a node
      // if (is_nodal)
      //  dof_object_ptr = to_mesh.node_ptr(dof_object_id);
      // It is an element
      // else
      //  dof_object_ptr = to_mesh.elem_ptr(dof_object_id);

      // We should only be supporting nodal and constant elemental
      // variables in this code path; if we see multiple DoFs on one
      // object we should have been using GenericProjector
      // mooseAssert(dof_object_ptr->n_dofs(sys_num, var_num) == 1,
      //            "Unexpectedly found " << dof_object_ptr->n_dofs(sys_num, var_num)
      //                                  << "dofs instead of 1");

      auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

      // Check if we visited this dof object ealier
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
        // We adopt values from the smallest rank which has a valid value
        if (val.distance > incoming_vals[val_offset].second ||
            (val.pid > pid && val.distance == incoming_vals[val_offset].second))
        {
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
    auto & es = _to_problems[problem_id]->es();

    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    MeshBase & to_mesh = _to_meshes[problem_id]->getMesh();
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

        DofObject * dof_object = nullptr;
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
    const auto & from_mesh = _from_meshes[j];

    std::set<SubdomainID> subdomains;
    if (isParamValid("from_blocks"))
    {
      // User input block names
      auto & blocks = getParam<std::vector<SubdomainName>>("from_blocks");
      // Subdomain ids
      std::vector<SubdomainID> ids = from_mesh->getSubdomainIDs(blocks);
      // Store these ids
      subdomains.insert(ids.begin(), ids.end());
    }

    std::set<BoundaryID> boundaries;
    if (isParamValid("from_boundaries"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundaries");
      std::vector<BoundaryID> boundary_ids = from_mesh->getBoundaryIDs(boundary_names);
      // Store these ids
      boundaries.insert(boundary_ids.begin(), boundary_ids.end());
    }

    for (auto & elem : as_range(from_mesh->getMesh().local_elements_begin(),
                                from_mesh->getMesh().local_elements_end()))
    {
      if (!subdomains.empty() && !hasBlocks(subdomains, elem))
        continue;

      for (auto & node : elem->node_ref_range())
      {
        if (!boundaries.empty() && !hasBoundaries(boundaries, *from_mesh, &node))
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

  return bboxes;
}
