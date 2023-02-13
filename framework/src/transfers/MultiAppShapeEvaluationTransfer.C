//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppShapeEvaluationTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and receive stuff

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", MultiAppShapeEvaluationTransfer);
registerMooseObjectRenamed("MooseApp",
                           MultiAppMeshFunctionTransfer,
                           "12/31/2023 24:00",
                           MultiAppShapeEvaluationTransfer);

InputParameters
MultiAppShapeEvaluationTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Transfers field data at the MultiApp position using solution the finite element function "
      "from the main/parent application, via a 'libMesh::MeshFunction' object.");

  params.addParam<bool>(
      "error_on_miss",
      false,
      "Whether or not to error in the case that a target point is not found in the source domain.");
  MultiAppTransfer::addBBoxFactorParam(params);
  return params;
}

MultiAppShapeEvaluationTransfer::MultiAppShapeEvaluationTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters), _error_on_miss(getParam<bool>("error_on_miss"))
{
  if (_to_var_names.size() == _from_var_names.size())
    _var_size = _to_var_names.size();
  else
    paramError("variable", "The number of variables to transfer to and from should be equal");
}

void
MultiAppShapeEvaluationTransfer::execute()
{
  TIME_SECTION("MultiAppShapeEvaluationTransfer::execute()",
               5,
               "Transferring variables via finite element interpolation");

  // loop over the vector of variables and make the transfer one by one
  for (unsigned int i = 0; i < _var_size; ++i)
    transferVariable(i);

  postExecute();
}

void
MultiAppShapeEvaluationTransfer::transferVariable(unsigned int i)
{
  mooseAssert(i < _var_size, "The variable of index " << i << " does not exist");

  /**
   * For every combination of global "from" problem and local "to" problem, find
   * which "from" bounding boxes overlap with which "to" elements.  Keep track
   * of which processors own bounding boxes that overlap with which elements.
   * Build vectors of node locations/element centroids to send to other
   * processors for mesh function evaluations.
   */

  // Get the bounding boxes for the "from" domains.
  std::vector<BoundingBox> bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  // Point locations needed to send to from-domain
  // processor to points
  std::map<processor_id_type, std::vector<Point>> outgoing_points;
  // <processor, <system_id, node_i>> --> point_id
  std::map<processor_id_type, std::map<std::pair<unsigned int, dof_id_type>, dof_id_type>>
      point_index_map;

  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    System * to_sys = find_sys(*_to_es[i_to], _to_var_names[i]);
    unsigned int sys_num = to_sys->number();
    unsigned int var_num = to_sys->variable_number(_to_var_names[i]);
    MeshBase * to_mesh = &_to_meshes[i_to]->getMesh();
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_constant = fe_type.order == CONSTANT;
    bool is_nodal = fe_type.family == LAGRANGE;
    const auto to_global_num = _current_direction == FROM_MULTIAPP ? 0 : _to_local2global_map[i_to];
    const auto & to_transform = *_to_transforms[to_global_num];

    if (fe_type.order > FIRST && !is_nodal)
      mooseError("We don't currently support second order or higher elemental variable.");

    if (is_nodal)
    {
      for (const auto & node : to_mesh->local_node_ptr_range())
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // Loop over the "froms" on processor i_proc.  If the node is found in
        // any of the "froms", add that node to the vector that will be sent to
        // i_proc.
        unsigned int from0 = 0;
        for (processor_id_type i_proc = 0; i_proc < n_processors();
             from0 += froms_per_proc[i_proc], ++i_proc)
        {
          bool point_found = false;
          for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && !point_found;
               ++i_from)
          {
            auto transformed_node = to_transform(*node);
            if (bboxes[i_from].contains_point(transformed_node))
            {
              // <system id, node id>
              std::pair<unsigned int, dof_id_type> key(i_to, node->id());
              // map a tuple of pid, problem id and node id to point id
              // point id is counted from zero
              point_index_map[i_proc][key] = outgoing_points[i_proc].size();
              // map pid to points
              outgoing_points[i_proc].push_back(std::move(transformed_node));
              point_found = true;
            }
          }
        }
      }
    }
    else // Elemental
    {
      std::vector<Point> points;
      std::vector<dof_id_type> point_ids;
      for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        points.clear();
        point_ids.clear();
        // grab sample points
        // for constant shape function, we take the element centroid
        if (is_constant)
        {
          points.push_back(elem->vertex_average());
          point_ids.push_back(elem->id());
        }

        // for higher order method, we take all nodes of element
        // this works for the first order L2 Lagrange.
        else
          for (auto & node : elem->node_ref_range())
          {
            points.push_back(node);
            point_ids.push_back(node.id());
          }

        unsigned int offset = 0;
        for (auto & point : points)
        {
          // Loop over the "froms" on processor i_proc.  If the elem is found in
          // any of the "froms", add that elem to the vector that will be sent to
          // i_proc.
          unsigned int from0 = 0;
          for (processor_id_type i_proc = 0; i_proc < n_processors();
               from0 += froms_per_proc[i_proc], ++i_proc)
          {
            bool point_found = false;
            for (unsigned int i_from = from0;
                 i_from < from0 + froms_per_proc[i_proc] && !point_found;
                 ++i_from)
            {
              auto transformed_point = to_transform(point);
              if (bboxes[i_from].contains_point(transformed_point))
              {
                std::pair<unsigned int, dof_id_type> key(i_to, point_ids[offset]);
                if (point_index_map[i_proc].find(key) != point_index_map[i_proc].end())
                  continue;

                point_index_map[i_proc][key] = outgoing_points[i_proc].size();
                outgoing_points[i_proc].push_back(std::move(transformed_point));
                point_found = true;
              } // if
            }   // i_from
          }     //  i_proc
          offset++;
        } // point

      } // else
    }
  }

  // Get the local bounding boxes for current processor.
  // There could be more than one box because of the number of local apps
  // can be larger than one
  std::vector<BoundingBox> local_bboxes(froms_per_proc[processor_id()]);
  {
    // Find the index to the first of this processor's local bounding boxes.
    unsigned int local_start = 0;
    for (processor_id_type i_proc = 0; i_proc < n_processors() && i_proc != processor_id();
         ++i_proc)
    {
      local_start += froms_per_proc[i_proc];
    }

    // Extract the local bounding boxes.
    for (unsigned int i_from = 0; i_from < froms_per_proc[processor_id()]; ++i_from)
    {
      local_bboxes[i_from] = bboxes[local_start + i_from];
    }
  }

  // Setup the local mesh functions.
  std::vector<MeshFunction> local_meshfuns;
  local_meshfuns.reserve(_from_problems.size());
  for (unsigned int i_from = 0; i_from < _from_problems.size(); ++i_from)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFEBase & from_var =
        from_problem.getVariable(0,
                                 _from_var_names[i],
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    local_meshfuns.emplace_back(getEquationSystem(from_problem, _displaced_source_mesh),
                                *from_sys.current_local_solution,
                                from_sys.get_dof_map(),
                                from_var_num);
    local_meshfuns.back().init();
    local_meshfuns.back().enable_out_of_mesh_mode(OutOfMeshValue);
  }

  /**
   * Gather all of the evaluations, pick out the best ones for each point, and
   * apply them to the solution vector.  When we are transferring from
   * multiapps, there may be multiple overlapping apps for a particular point.
   * In that case, we'll try to use the value from the app with the lowest id.
   */

  // Fill values and app ids for incoming points
  // We are responsible to compute values for these incoming points
  auto gather_functor =
      [this, &local_meshfuns, &local_bboxes](
          processor_id_type /*pid*/,
          const std::vector<Point> & incoming_points,
          std::vector<std::pair<Real, unsigned int>> & vals_ids_for_incoming_points)
  {
    vals_ids_for_incoming_points.resize(incoming_points.size(), std::make_pair(OutOfMeshValue, 0));
    for (MooseIndex(incoming_points.size()) i_pt = 0; i_pt < incoming_points.size(); ++i_pt)
    {
      Point pt = incoming_points[i_pt];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (MooseIndex(_from_problems.size()) i_from = 0;
           i_from < _from_problems.size() &&
           vals_ids_for_incoming_points[i_pt].first == OutOfMeshValue;
           ++i_from)
      {
        if (local_bboxes[i_from].contains_point(pt))
        {
          const auto from_global_num =
              _current_direction == TO_MULTIAPP ? 0 : _from_local2global_map[i_from];
          // Use mesh funciton to compute interpolation values
          vals_ids_for_incoming_points[i_pt].first =
              (local_meshfuns[i_from])(_from_transforms[from_global_num]->mapBack(pt));
          // Record problem ID as well
          switch (_current_direction)
          {
            case FROM_MULTIAPP:
              vals_ids_for_incoming_points[i_pt].second = _from_local2global_map[i_from];
              break;
            case TO_MULTIAPP:
              vals_ids_for_incoming_points[i_pt].second = _to_local2global_map[i_from];
              break;
            default:
              mooseError("Unsupported direction");
          }
        }
      }
    }
  };

  // Incoming values and APP ids for outgoing points
  std::map<processor_id_type, std::vector<std::pair<Real, unsigned int>>> incoming_vals_ids;
  // Copy data out to incoming_vals_ids
  auto action_functor =
      [&incoming_vals_ids](
          processor_id_type pid,
          const std::vector<Point> & /*my_outgoing_points*/,
          const std::vector<std::pair<Real, unsigned int>> & vals_ids_for_outgoing_points)
  {
    // This lambda function might be called multiple times
    incoming_vals_ids[pid].reserve(vals_ids_for_outgoing_points.size());
    // Copy data for processor 'pid'
    std::copy(vals_ids_for_outgoing_points.begin(),
              vals_ids_for_outgoing_points.end(),
              std::back_inserter(incoming_vals_ids[pid]));
  };

  // We assume incoming_vals_ids is ordered in the same way as outgoing_points
  // Hopefully, pull_parallel_vector_data will not mess up this
  const std::pair<Real, unsigned int> * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      comm(), outgoing_points, gather_functor, action_functor, ex);

  for (unsigned int i_to = 0; i_to < _to_problems.size(); ++i_to)
  {
    const auto to_global_num = _current_direction == FROM_MULTIAPP ? 0 : _to_local2global_map[i_to];
    System * to_sys = find_sys(*_to_es[i_to], _to_var_names[i]);

    unsigned int sys_num = to_sys->number();
    unsigned int var_num = to_sys->variable_number(_to_var_names[i]);

    NumericVector<Real> * solution = nullptr;
    switch (_current_direction)
    {
      case TO_MULTIAPP:
        solution = &getTransferVector(i_to, _to_var_names[i]);
        break;
      case FROM_MULTIAPP:
        solution = to_sys->solution.get();
        break;
      default:
        mooseError("Unknown direction");
    }

    MeshBase * to_mesh = &_to_meshes[i_to]->getMesh();
    auto & fe_type = to_sys->variable_type(var_num);
    bool is_constant = fe_type.order == CONSTANT;
    bool is_nodal = fe_type.family == LAGRANGE;

    if (is_nodal)
    {
      for (const auto & node : to_mesh->local_node_ptr_range())
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        unsigned int lowest_app_rank = libMesh::invalid_uint;
        Real best_val = 0.;
        bool point_found = false;
        for (auto & group : incoming_vals_ids)
        {
          // Skip this proc if the node wasn't in it's bounding boxes.
          std::pair<unsigned int, dof_id_type> key(i_to, node->id());
          // Make sure point_index_map has data for corresponding pid
          mooseAssert(point_index_map.find(group.first) != point_index_map.end(),
                      "Point index map does not have data for processor group.first");
          if (point_index_map[group.first].find(key) == point_index_map[group.first].end())
            continue;

          auto i_pt = point_index_map[group.first][key];

          // Ignore this proc if it's app has a higher rank than the
          // previously found lowest app rank.
          if (_current_direction == FROM_MULTIAPP)
          {
            if (group.second[i_pt].second >= lowest_app_rank)
              continue;
          }

          // Ignore this proc if the point was actually outside its meshes.
          if (group.second[i_pt].first == OutOfMeshValue)
            continue;

          best_val = group.second[i_pt].first;
          point_found = true;
        }

        if (_error_on_miss && !point_found)
          mooseError("Point not found in the reference space! ",
                     (*_to_transforms[to_global_num])(*node));

        dof_id_type dof = node->dof_number(sys_num, var_num, 0);
        solution->set(dof, best_val);
      }
    }
    else // Elemental
    {
      std::vector<Point> points;
      std::vector<dof_id_type> point_ids;
      for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        points.clear();
        point_ids.clear();
        // grap sample points
        // for constant shape function, we take the element centroid
        if (is_constant)
        {
          points.push_back(elem->vertex_average());
          point_ids.push_back(elem->id());
        }
        // for higher order method, we take all nodes of element
        // this works for the first order L2 Lagrange. Might not work
        // with something higher than the first order
        else
        {
          for (auto & node : elem->node_ref_range())
          {
            points.push_back(node);
            point_ids.push_back(node.id());
          }
        }

        auto n_points = points.size();
        unsigned int n_comp = elem->n_comp(sys_num, var_num);
        // We assume each point corresponds to one component of elemental variable
        if (n_points != n_comp)
          mooseError(" Number of points ",
                     n_points,
                     " does not equal to number of variable components ",
                     n_comp);
        for (unsigned int offset = 0; offset < n_points; offset++)
        {
          unsigned int lowest_app_rank = libMesh::invalid_uint;
          Real best_val = 0;
          bool point_found = false;
          for (auto & group : incoming_vals_ids)
          {
            // Skip this proc if the elem wasn't in it's bounding boxes.
            std::pair<unsigned int, dof_id_type> key(i_to, point_ids[offset]);
            if (point_index_map[group.first].find(key) == point_index_map[group.first].end())
              continue;

            unsigned int i_pt = point_index_map[group.first][key];

            // Ignore this proc if it's app has a higher rank than the
            // previously found lowest app rank.
            if (_current_direction == FROM_MULTIAPP)
            {
              if (group.second[i_pt].second >= lowest_app_rank)
                continue;
            }

            // Ignore this proc if the point was actually outside its meshes.
            if (group.second[i_pt].first == OutOfMeshValue)
              continue;

            best_val = group.second[i_pt].first;
            point_found = true;
          }

          if (_error_on_miss && !point_found)
            mooseError("Point not found in the reference space! ",
                       (*_to_transforms[to_global_num])(elem->vertex_average()));

          // Get the value for a dof
          dof_id_type dof = elem->dof_number(sys_num, var_num, offset);
          solution->set(dof, best_val);
        } // point
      }   // element
    }
    solution->close();
    to_sys->update();
  }
}
