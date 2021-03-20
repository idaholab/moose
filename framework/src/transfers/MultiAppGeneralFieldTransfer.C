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

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", MultiAppGeneralFieldTransfer);

InputParameters
MultiAppGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position using solution the finite element function "
      "from the master application, via a 'libMesh::MeshFunction' object.");

  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");

  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "The blocks we are transferring to (if not specified, whole domain is used).");

  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "The blocks we are transferring from (if not specified, whole domain is used).");

  return params;
}

MultiAppGeneralFieldTransfer::MultiAppGeneralFieldTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters), _error_on_miss(getParam<bool>("error_on_miss"))
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
MultiAppGeneralFieldTransfer::locatePointReceivers(const Point point,
                                                   std::vector<processor_id_type> & processors)
{
  // Check which processors include this point
  // One point might have more than one points
  bool found = false;
  unsigned int from0 = 0;
  for (processor_id_type i_proc = 0; i_proc < n_processors();
       from0 += _froms_per_proc[i_proc], ++i_proc)
    for (unsigned int i_from = from0; i_from < from0 + _froms_per_proc[i_proc]; ++i_from)
      if (_bboxes[i_from].contains_point(point))
      {
        processors.push_back(i_from);
        found = true;
      }

  // Error out if we could not find this point when ask us to do so
  if (!found && _error_on_miss)
    mooseError("Cannot locate point ", point, " \n ", "mismatched meshes are used");
}

void
MultiAppGeneralFieldTransfer::extractOutgoingPoints(const VariableName & var_name,
                                                    ProcessorToPointVec & outgoing_points)
{
  // Clean up to blocks that were cached
  _to_blocks.clear();
  // Clean up the map from processor to pointInfo vector
  // This map shuld be consistent with outgoing_points
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

    // We do not support high-order elemental variables
    if (fe_type.order > CONSTANT && !is_nodal)
      mooseError("We don't currently support first order or higher elemental variable ");

    // Receivers for a given point
    std::vector<processor_id_type> processors;

    if (is_nodal)
    {
      for (const auto & node : to_mesh.local_node_ptr_range())
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // Skip if it is a block restricted transfer and current node does not have
        // specified blocks
        if (blockRestrictedTarget() && !hasBlocks(_to_blocks, to_moose_mesh, node))
          continue;

        // Try to find which processors
        processors.clear();
        locatePointReceivers((*node + _to_positions[i_to]), processors);

        // We need to send these data to these processors
        for (auto pid : processors)
        {
          outgoing_points[pid].push_back((*node + _to_positions[i_to]));
          // Store point information
          // We can use these information when insert values to solution vector
          PointInfo pointinfo;
          pointinfo.problem_id = i_to;
          pointinfo.dof_object_id = node->id();
          pointinfo.offset = 0;
          _processor_to_pointInfoVec[pid].push_back(pointinfo);
        }
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
        if (blockRestrictedTarget() && !hasBlocks(_to_blocks, elem))
          continue;

        // Try to find which processors
        processors.clear();
        // Right now, let use consider constant elemental variables only
        // We need to attach qp points for higher order elemental variables
        locatePointReceivers(elem->centroid() + _to_positions[i_to], processors);

        // We need to send these data to these processors
        for (auto pid : processors)
        {
          outgoing_points[pid].push_back(elem->centroid() + _to_positions[i_to]);
          // Store point information
          // We can use these information when insert values to solution vector
          PointInfo pointinfo;
          pointinfo.problem_id = i_to;
          pointinfo.dof_object_id = elem->id();
          // We need to update this for high order elemental variables
          pointinfo.offset = 0;
          _processor_to_pointInfoVec[pid].push_back(pointinfo);
        } // for
      }   // for
    }     // else
  }       // for
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
MultiAppGeneralFieldTransfer::buildMeshFunctions(
    const VariableName & var_name, std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns)
{
  // Construct a local mesh for each problem
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    std::shared_ptr<MeshFunction> from_func;
    from_func.reset(new MeshFunction(
        from_problem.es(), *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num));
    from_func->init(Trees::ELEMENTS);
    from_func->enable_out_of_mesh_mode(OutOfMeshValue);
    local_meshfuns.push_back(from_func);
  }
}

void
MultiAppGeneralFieldTransfer::evaluateInterpValues(
    const std::vector<BoundingBox> & local_bboxes,
    const std::vector<std::shared_ptr<MeshFunction>> & local_meshfuns,
    const std::vector<Point> & incoming_points,
    std::vector<Real> & outgoing_vals)
{
  dof_id_type i_pt = 0;
  for (auto & pt : incoming_points)
  {
    // Loop until we've found the lowest-ranked app that actually contains
    // the quadrature point.
    for (MooseIndex(_from_problems.size()) i_from = 0;
         i_from < _from_problems.size() && outgoing_vals[i_pt] == OutOfMeshValue;
         ++i_from)
    {
      if (local_bboxes[i_from].contains_point(pt))
      {
        // Use mesh funciton to compute interpolation values
        auto val = (*local_meshfuns[i_from])(pt - _from_positions[i_from]);
        // Assign value
        outgoing_vals[i_pt] = val;
      }
    }

    // Move to next point
    i_pt++;
  }
}

void
MultiAppGeneralFieldTransfer::cacheIncomingInterpVals(
    processor_id_type pid,
    const VariableName & var_name,
    std::vector<PointInfo> & pointInfoVec,
    const std::vector<Real> & incoming_vals,
    DofobjectToInterpValVec & dofobject_to_valsvec)
{
  mooseAssert(pointInfoVec.size() == incoming_vals.size(),
              " Number of dof objects does not equal to the number of incoming values");

  dof_id_type val_offset = 0;
  for (auto & pointinfo : pointInfoVec)
  {
    auto problem_id = pointinfo.problem_id;
    auto dof_object_id = pointinfo.dof_object_id;

    std::pair<unsigned int, dof_id_type> dofobject(problem_id, dof_object_id);

    // libMesh EquationSystems
    auto & es = _to_problems[problem_id]->es();
    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    MeshBase * to_mesh = &_to_meshes[problem_id]->getMesh();
    auto var_num = to_sys->variable_number(var_name);
    auto sys_num = to_sys->number();
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    if (is_nodal)
    {
      auto & node = to_mesh->node_ref(dof_object_id);
      auto n_dofs = node.n_dofs(sys_num, var_num);
      auto values_ptr = dofobject_to_valsvec.find(dofobject);

      if (values_ptr == dofobject_to_valsvec.end())
      {
        auto & val_vec = dofobject_to_valsvec[dofobject];
        val_vec.resize(n_dofs);
        val_vec[0].first = incoming_vals[val_offset];
        val_vec[0].second = pid;
      }
      else
      {
        auto & val_vec = dofobject_to_valsvec[dofobject];
        if ((val_vec[0].second > pid || val_vec[0].first == OutOfMeshValue) &&
            incoming_vals[val_offset] != OutOfMeshValue)
          val_vec[0].first = incoming_vals[val_offset];
      }
    }
    else
    {
      auto & element = to_mesh->elem_ref(dof_object_id);
      auto n_dofs = element.n_dofs(sys_num, var_num);
      auto values_ptr = dofobject_to_valsvec.find(dofobject);

      if (values_ptr == dofobject_to_valsvec.end())
      {
        auto & val_vec = dofobject_to_valsvec[dofobject];
        val_vec.resize(n_dofs);
        val_vec[0].first = incoming_vals[val_offset];
        val_vec[0].second = pid;
      }
      else
      {
        auto & val_vec = dofobject_to_valsvec[dofobject];
        if ((val_vec[0].second > pid || val_vec[0].first == OutOfMeshValue) &&
            incoming_vals[val_offset] != OutOfMeshValue)
          val_vec[0].first = incoming_vals[val_offset];
      }
    }

    val_offset++;
  }
}

void
MultiAppGeneralFieldTransfer::getToSolutionVector(const VariableName & var,
                                                  unsigned int to_problem,
                                                  System & to_sys,
                                                  NumericVector<Number> ** solution)
{
  switch (_current_direction)
  {
    case TO_MULTIAPP:
      *solution = &getTransferVector(to_problem, var);
      break;
    case FROM_MULTIAPP:
      *solution = to_sys.solution.get();
      break;
    default:
      mooseError("Unknown direction");
  }
}

void
MultiAppGeneralFieldTransfer::setSolutionVectorValues(
    const VariableName & var_name, DofobjectToInterpValVec & dofobject_to_valsvec)
{
  for (auto & id_pair : dofobject_to_valsvec)
  {
    auto problem_id = id_pair.first.first;
    auto dof_object_id = id_pair.first.second;

    // libMesh EquationSystems
    auto & es = _to_problems[problem_id]->es();
    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    MeshBase * to_mesh = &_to_meshes[problem_id]->getMesh();
    auto var_num = to_sys->variable_number(var_name);
    auto sys_num = to_sys->number();

    NumericVector<Real> * solution = nullptr;
    getToSolutionVector(var_name, problem_id, *to_sys, &solution);
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_nodal = fe_type.family == LAGRANGE;

    dof_id_type dof = -1;
    if (is_nodal)
    {
      auto & dof_object = to_mesh->node_ref(dof_object_id);
      dof = dof_object.dof_number(sys_num, var_num, 0);
    }
    else
    {
      auto & dof_object = to_mesh->elem_ref(dof_object_id);
      dof = dof_object.dof_number(sys_num, var_num, 0);
    }
    // If there are more than one dofs for this dof object,
    // we need to add offset
    auto val = id_pair.second[0].first;

    solution->set(dof, val);
  }

  // Update solution and sync solution
  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    System * to_sys = find_sys(*_to_es[i_to], var_name);

    NumericVector<Real> * solution = nullptr;
    getToSolutionVector(var_name, i_to, *to_sys, &solution);

    solution->close();
    to_sys->update();
  }
}

void
MultiAppGeneralFieldTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

  // Get the bounding boxes for the "from" domains.
  _bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
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
  std::vector<std::shared_ptr<MeshFunction>> local_meshfuns;
  buildMeshFunctions(_from_var_names[i], local_meshfuns);

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor =
      [this, &local_bboxes, &local_meshfuns](processor_id_type /*pid*/,
                                             const std::vector<Point> & incoming_points,
                                             std::vector<Real> & outgoing_vals) {
        outgoing_vals.resize(incoming_points.size(), OutOfMeshValue);
        // Evaluate interpolation values for these incoming points
        evaluateInterpValues(local_bboxes, local_meshfuns, incoming_points, outgoing_vals);
      };

  DofobjectToInterpValVec dofobject_to_valsvec;
  // Copy data out to incoming_vals_ids
  auto action_functor =
      [this, &i, &dofobject_to_valsvec](processor_id_type pid,
                                        const std::vector<Point> & /*my_outgoing_points*/,
                                        const std::vector<Real> & incoming_vals) {
        auto & pointInfoVec = _processor_to_pointInfoVec[pid];

        cacheIncomingInterpVals(
            pid, _to_var_names[i], pointInfoVec, incoming_vals, dofobject_to_valsvec);
      };

  // We assume incoming_vals_ids is ordered in the same way as outgoing_points
  // Hopefully, pull_parallel_vector_data will not mess up this
  const Real * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, ex);

  setSolutionVectorValues(_to_var_names[i], dofobject_to_valsvec);
}

bool
MultiAppGeneralFieldTransfer::blockRestrictedTarget() const
{
  return !_to_blocks.empty();
}

bool
MultiAppGeneralFieldTransfer::blockRestrictedSource() const
{
  return !_from_blocks.empty();
}

bool
MultiAppGeneralFieldTransfer::hasBlocks(std::set<SubdomainID> & blocks, const Elem * elem) const
{
  return blocks.find(elem->subdomain_id()) != blocks.end();
}

bool
MultiAppGeneralFieldTransfer::hasBlocks(std::set<SubdomainID> & blocks,
                                        const MooseMesh * mesh,
                                        const Node * node) const
{
  const std::set<SubdomainID> & node_blocks = mesh->getNodeBlockIds(*node);
  std::set<SubdomainID> u;
  std::set_intersection(blocks.begin(),
                        blocks.end(),
                        node_blocks.begin(),
                        node_blocks.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}
