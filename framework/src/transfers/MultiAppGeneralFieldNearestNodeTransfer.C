//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeneralFieldNearestNodeTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"

#include "libmesh/generic_projector.h"
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

// Anonymous namespace for data, functors to use with GenericProjector.
namespace NearestNode
{

// Transfer::OutOfMeshValue is an actual number.  Why?  Why!
static_assert(std::numeric_limits<Real>::has_infinity,
              "What are you trying to use for Real?  It lacks infinity!");
Number BetterOutOfMeshValue = std::numeric_limits<Real>::infinity();

inline bool
isBetterOutOfMeshValue(Number val)
{
  // Might need to be changed for e.g. NaN
  return val == NearestNode::BetterOutOfMeshValue;
}

// We need two functors that record point (value and gradient,
// respectively) requests, so we know what queries we need to make
// to other processors

/**
 * Value request recording base class
 */
template <typename Output>
class RecordRequests
{
protected:
  typedef typename TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef typename TensorTools::MakeReal<Output>::type RealType;
  typedef DofValueType ValuePushType;
  typedef Output FunctorValue;

  RecordRequests() {}

  RecordRequests(RecordRequests & primary) : _primary(&primary) {}

  ~RecordRequests()
  {
    if (_primary)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _primary->_points_requested.insert(
          _primary->_points_requested.end(), _points_requested.begin(), _points_requested.end());
    }
  }

  void init_context(FEMContext &) {}

  Output eval_at_node(const FEMContext &,
                      unsigned int libmesh_dbg_var(i),
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    libmesh_assert_not_equal_to(i, 0);
    _points_requested.push_back(n);
    return 0;
  }

  Output eval_at_point(const FEMContext &,
                       unsigned int libmesh_dbg_var(i),
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    libmesh_assert_not_equal_to(i, 0);
    _points_requested.push_back(n);
    return 0;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    libmesh_error();
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    libmesh_error();
  }

  void eval_old_dofs(const Elem &,
                     const FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    libmesh_error();
  }

  std::vector<Point> & points_requested() { return _points_requested; }

private:
  std::vector<Point> _points_requested;

  RecordRequests * _primary = nullptr;
};

// We need a null action functor to use
// with them (because we won't be ready to set any values at that
// point)
template <typename Val>
class NullAction
{
public:
  typedef Val InsertInput;

  NullAction() {}

  void insert(dof_id_type, Val) {}

  void insert(const std::vector<dof_id_type> &, const DenseVector<Val> &) {}
};

// We need two functors that respond to point (value and gradient,
// respectively) requests based on the cached values of queries answered by
// other processors.

/**
 * Value request response base class
 */
template <typename Output>
class CachedData
{
protected:
  typedef typename TensorTools::MakeBaseNumber<Output>::type DofValueType;

public:
  typedef std::unordered_map<Point, Output, MultiAppGeneralFieldNearestNodeTransfer::hash_point>
      Cache;

  typedef typename TensorTools::MakeReal<Output>::type RealType;
  typedef DofValueType ValuePushType;
  typedef Output FunctorValue;

  CachedData(const Cache & cache, const FunctionBase<Output> & backup)
    : _cache(cache), _backup(backup.clone())
  {
  }

  CachedData(const CachedData & primary) : _cache(primary._cache), _backup(primary._backup->clone())
  {
  }

  void init_context(FEMContext &) {}

  Output eval_at_node(const FEMContext &,
                      unsigned int /*i*/,
                      unsigned int /*elem_dim*/,
                      const Node & n,
                      bool /*extra_hanging_dofs*/,
                      const Real /*time*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
      return (*_backup)(n);
    else
      return it->second;
  }

  Output eval_at_point(const FEMContext &,
                       unsigned int /*i*/,
                       const Point & n,
                       const Real /*time*/,
                       bool /*skip_context_check*/)
  {
    auto it = _cache.find(n);
    if (it == _cache.end())
      return (*_backup)(n);
    else
      return it->second;
  }

  bool is_grid_projection() { return false; }

  void eval_mixed_derivatives(const FEMContext & /*c*/,
                              unsigned int /*i*/,
                              unsigned int /*dim*/,
                              const Node & /*n*/,
                              std::vector<Output> & /*derivs*/)
  {
    libmesh_error();
  } // this is only for grid projections

  void eval_old_dofs(
      const Elem &, unsigned int, unsigned int, std::vector<dof_id_type> &, std::vector<Output> &)
  {
    libmesh_error();
  }

  void eval_old_dofs(const Elem &,
                     const FEType &,
                     unsigned int,
                     unsigned int,
                     std::vector<dof_id_type> &,
                     std::vector<Output> &)
  {
    libmesh_error();
  }

private:
  // Data to return for cached points
  const Cache & _cache;

  // Function to evaluate for uncached points
  std::unique_ptr<FunctionBase<Output>> _backup;
};

}

registerMooseObject("MooseApp", MultiAppGeneralFieldNearestNodeTransfer);

InputParameters
MultiAppGeneralFieldNearestNodeTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position using solution the finite element function "
      "from the master application, via a 'libMesh::MeshFunction' object.");

  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");

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

  params.addParam<Real>("bbox_tol", 0.1, "How much want to relax bounding boxes");

  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "The blocks we are transferring to (if not specified, whole domain is used).");

  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "The blocks we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "from_boundary",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "to_boundary",
      "The boundary we are transferring to (if not specified, whole domain is used).");

  return params;
}

MultiAppGeneralFieldNearestNodeTransfer::MultiAppGeneralFieldNearestNodeTransfer(
    const InputParameters & parameters)
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
MultiAppGeneralFieldNearestNodeTransfer::execute()
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
MultiAppGeneralFieldNearestNodeTransfer::transferVariable(unsigned int i)
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

  // Get the local bounding boxes for current processor.
  // There could be more than one box because of the number of local apps
  // can be larger than one
  std::vector<BoundingBox> local_bboxes;
  extractLocalFromBoundingBoxes(local_bboxes);

  // Setup the local mesh functions.
  std::vector<std::shared_ptr<KDTree>> local_kdtrees;
  std::vector<std::vector<Point>> local_points;
  std::vector<std::vector<Real>> local_values;

  buildKDTrees(_from_var_names[i], local_kdtrees, local_points, local_values);

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor = [this, &local_kdtrees, &local_points, &local_values](
                            processor_id_type /*pid*/,
                            const std::vector<Point> & incoming_points,
                            std::vector<std::pair<Real, Real>> & outgoing_vals)
  {
    outgoing_vals.resize(incoming_points.size(),
                         {NearestNode::BetterOutOfMeshValue, NearestNode::BetterOutOfMeshValue});
    // Evaluate interpolation values for these incoming points
    evaluateInterpValues(local_kdtrees, local_points, local_values, incoming_points, outgoing_vals);
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
MultiAppGeneralFieldNearestNodeTransfer::locatePointReceivers(
    const Point point, std::set<processor_id_type> & processors)
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
MultiAppGeneralFieldNearestNodeTransfer::cacheOutgoingPointInfor(
    const Point point,
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
MultiAppGeneralFieldNearestNodeTransfer::extractOutgoingPoints(
    const VariableName & var_name, ProcessorToPointVec & outgoing_points)
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
    if (isParamValid("to_boundary"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("to_boundary");
      std::vector<BoundaryID> boundary_ids = to_moose_mesh->getBoundaryIDs(boundary_names);
      // Store these ids
      _to_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
    }

    // We support more general variables via libMesh GenericProjector
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      NearestNode::RecordRequests<Number> f;
      NearestNode::RecordRequests<Gradient> g;
      NearestNode::NullAction<Number> nullsetter;
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<NearestNode::RecordRequests<Number>,
                                NearestNode::RecordRequests<Gradient>,
                                Number,
                                NearestNode::NullAction<Number>>
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
MultiAppGeneralFieldNearestNodeTransfer::extractLocalFromBoundingBoxes(
    std::vector<BoundingBox> & local_bboxes)
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
MultiAppGeneralFieldNearestNodeTransfer::buildKDTrees(
    const VariableName & var_name,
    std::vector<std::shared_ptr<KDTree>> & kdtrees,
    std::vector<std::vector<Point>> & points,
    std::vector<std::vector<Real>> & local_values)
{
  kdtrees.resize(_from_problems.size());
  points.resize(_from_problems.size());
  local_values.resize(_from_problems.size());

  // Construct a local mesh for each problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    auto & from_mesh = from_problem.mesh();
    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());
    // FEM type info
    auto & fe_type = from_sys.variable_type(from_var.number());
    bool is_nodal = fe_type.family == LAGRANGE;

    std::set<SubdomainID> from_blocks;
    // Take users' input block names
    // Change them to ids
    // Store then in a member variables
    if (isParamValid("from_blocks"))
    {
      // User input block names
      auto & blocks = getParam<std::vector<SubdomainName>>("from_blocks");
      // Subdomain ids
      std::vector<SubdomainID> ids = from_mesh.getSubdomainIDs(blocks);
      // We might have more than one problems
      from_blocks.clear();
      // Store these ids
      from_blocks.insert(ids.begin(), ids.end());
    }

    std::set<BoundaryID> from_boundaries;
    if (isParamValid("from_boundary"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundary");
      std::vector<BoundaryID> boundary_ids = from_mesh.getBoundaryIDs(boundary_names);
      // Store these ids
      from_boundaries.insert(boundary_ids.begin(), boundary_ids.end());
    }

    if (is_nodal)
    {
      for (const auto & node : from_mesh.getMesh().local_node_ptr_range())
      {
        if (node->n_dofs(from_sys.number(), from_var_num) < 1)
          continue;

        if (!from_blocks.empty() && !hasBlocks(from_blocks, from_mesh, node))
          continue;

        if (!from_boundaries.empty() && !hasBoundaries(from_boundaries, from_mesh, node))
          continue;

        points[i_from].push_back(*node + _from_positions[i_from]);
        auto dof = node->dof_number(from_sys.number(), from_var_num, 0);

        local_values[i_from].push_back((*from_sys.solution)(dof));
      }
    }
    else
    {
      if (!from_boundaries.empty())
      {
        mooseError("You can not restrict an elemental variable to boundary");
      }
      for (auto & elem : as_range(from_mesh.getMesh().local_elements_begin(),
                                  from_mesh.getMesh().local_elements_end()))
      {
        if (elem->n_dofs(from_sys.number(), from_var_num) < 1)
          continue;

        if (!from_blocks.empty() && !hasBlocks(from_blocks, elem))
          continue;

        auto dof = elem->dof_number(from_sys.number(), from_var_num, 0);
        points[i_from].push_back(elem->vertex_average() + _from_positions[i_from]);
        local_values[i_from].push_back((*from_sys.solution)(dof));
      }
    }

    std::shared_ptr<KDTree> _kd_tree =
        std::make_shared<KDTree>(points[i_from], from_mesh.getMaxLeafSize());
    kdtrees[i_from] = _kd_tree;
  }
}

void
MultiAppGeneralFieldNearestNodeTransfer::evaluateInterpValues(
    const std::vector<std::shared_ptr<KDTree>> & local_kdtrees,
    const std::vector<std::vector<Point>> & local_points,
    const std::vector<std::vector<Real>> & local_values,
    const std::vector<Point> & incoming_points,
    std::vector<std::pair<Real, Real>> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    // Loop until we've found the lowest-ranked app that actually contains
    // the quadrature point.
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() &&
         outgoing_vals[i_pt].first == NearestNode::BetterOutOfMeshValue;
         ++i_from)
    {

      std::vector<std::size_t> return_index(_num_nearest_points);
      std::vector<Real> return_dist_sqr(_num_nearest_points);
      if (local_kdtrees[i_from]->numberCandidatePoints())
      {
        local_kdtrees[i_from]->neighborSearch(
            pt, _num_nearest_points, return_index, return_dist_sqr);
        Real val_sum = 0, dist_sum = 0;
        for (auto index : return_index)
        {
          val_sum += local_values[i_from][index];
          dist_sum += (local_points[i_from][index] - pt).norm();
        }
        // Use mesh funciton to compute interpolation values
        // Assign value
        outgoing_vals[i_pt] = {val_sum / return_index.size(), dist_sum / return_dist_sqr.size()};
      }
      else
      {
        outgoing_vals[i_pt] = {NearestNode::BetterOutOfMeshValue,
                               NearestNode::BetterOutOfMeshValue};
      }
      /*Real min_distance = std::numeric_limits<Real>::max();
      dof_id_type min_id = 0, count = 0;
      for (auto point: local_points[i_from])
      {
         if (min_distance > (pt - point).norm())
         {
             min_distance = (pt - point).norm();
             min_id = count;
         }
         count++;
      }
      outgoing_vals[i_pt] = {local_values[i_from][min_id], min_distance}; */
    }

    // Move to next point
    i_pt++;
  }
}

void
MultiAppGeneralFieldNearestNodeTransfer::cacheIncomingInterpVals(
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
      if (!NearestNode::isBetterOutOfMeshValue(val))
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
MultiAppGeneralFieldNearestNodeTransfer::setSolutionVectorValues(
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

      NearestNode::CachedData<Number> f(interp_caches[problem_id], to_func);
      libMesh::VectorSetAction<Number> setter(*to_sys->solution);
      const std::vector<unsigned int> varvec(1, var_num);

      libMesh::GenericProjector<NearestNode::CachedData<Number>,
                                NearestNode::CachedData<Gradient>,
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
        if (_error_on_miss && NearestNode::isBetterOutOfMeshValue(val))
        {
          if (is_nodal)
            mooseError("Node ", dof_object_id, " for app ", problem_id, " could not be located ");
          else
            mooseError(
                "Element ", dof_object_id, " for app ", problem_id, " could not be located ");
        }

        // We should not put garbage into solution vector
        if (NearestNode::isBetterOutOfMeshValue(val))
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
MultiAppGeneralFieldNearestNodeTransfer::hasBlocks(std::set<SubdomainID> & blocks,
                                                   const Elem * elem) const
{
  return blocks.find(elem->subdomain_id()) != blocks.end();
}

bool
MultiAppGeneralFieldNearestNodeTransfer::hasBlocks(std::set<SubdomainID> & blocks,
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
MultiAppGeneralFieldNearestNodeTransfer::hasBoundaries(std::set<BoundaryID> & boundaries,
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
MultiAppGeneralFieldNearestNodeTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox)
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
MultiAppGeneralFieldNearestNodeTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox)
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
MultiAppGeneralFieldNearestNodeTransfer::getRestrictedFromBoundingBoxes()
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
    if (isParamValid("from_boundary"))
    {
      // User input block names
      auto & boundary_names = getParam<std::vector<BoundaryName>>("from_boundary");
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
