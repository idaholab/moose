//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppNearestNodeTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/dof_object.h"

// TIMPI includes
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", MultiAppNearestNodeTransfer);

InputParameters
MultiAppNearestNodeTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Transfer the value to the target domain from the nearest node in the source domain.");

  params.addParam<BoundaryName>(
      "source_boundary",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "target_boundary",
      "The boundary we are transferring to (if not specified, whole domain is used).");
  params.addParam<bool>("fixed_meshes",
                        false,
                        "Set to true when the meshes are not changing (ie, "
                        "no movement or adaptivity).  This will cache "
                        "nearest node neighbors to greatly speed up the "
                        "transfer.");

  MultiAppTransfer::addBBoxFactorParam(params);
  return params;
}

MultiAppNearestNodeTransfer::MultiAppNearestNodeTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _fixed_meshes(getParam<bool>("fixed_meshes")),
    _node_map(declareRestartableData<std::map<dof_id_type, Node *>>("node_map")),
    _distance_map(declareRestartableData<std::map<dof_id_type, Real>>("distance_map")),
    _neighbors_cached(declareRestartableData<bool>("neighbors_cached", false)),
    _cached_froms(declareRestartableData<std::map<processor_id_type, std::vector<unsigned int>>>(
        "cached_froms")),
    _cached_dof_ids(declareRestartableData<std::map<processor_id_type, std::vector<dof_id_type>>>(
        "cached_dof_ids")),
    _cached_from_inds(
        declareRestartableData<std::map<std::pair<unsigned int, dof_id_type>, unsigned int>>(
            "cached_from_ids")),
    _cached_qp_inds(
        declareRestartableData<std::map<std::pair<unsigned int, dof_id_type>, unsigned int>>(
            "cached_qp_inds"))
{
  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only");

  if (_from_var_names.size() != 1)
    paramError("source_variable", " Support single from-variable only");
}

void
MultiAppNearestNodeTransfer::execute()
{
  TIME_SECTION(
      "MultiAppNearestNodeTransfer::execute()", 5, "Transferring variables based on nearest nodes");

  // Get the bounding boxes for the "from" domains.
  std::vector<BoundingBox> bboxes;
  if (isParamValid("source_boundary"))
  {
    if (_from_meshes.size())
    {
      const auto & sb = getParam<BoundaryName>("source_boundary");
      if (!MooseMeshUtils::hasBoundaryName(_from_meshes[0]->getMesh(), sb))
        paramError("source_boundary", "The boundary '", sb, "' was not found in the mesh");

      bboxes = getFromBoundingBoxes(_from_meshes[0]->getBoundaryID(sb));
    }
    else
      bboxes = getFromBoundingBoxes(Moose::INVALID_BOUNDARY_ID);
  }
  else
    bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  ////////////////////
  // For every point in the local "to" domain, figure out which "from" domains
  // might contain its nearest neighbor, and send that point to the processors
  // that own those "from" domains.
  //
  // How do we know which "from" domains might contain the nearest neighbor, you
  // ask?  Well, consider two "from" domains, A and B.  If every point in A is
  // closer than every point in B, then we know that B cannot possibly contain
  // the nearest neighbor.  Hence, we'll only check A for the nearest neighbor.
  // We'll use the functions bboxMaxDistance and bboxMinDistance to figure out
  // if every point in A is closer than every point in B.
  ////////////////////

  // outgoing_qps = nodes/centroids we'll send to other processors.
  // Map processor to quadrature points. We will send these points to remote processors
  std::map<processor_id_type, std::vector<Point>> outgoing_qps;
  // When we get results back, node_index_map will tell us which results go with
  // which points
  // <processor, <system_id, node_i>> --> point_id
  std::map<processor_id_type, std::map<std::pair<unsigned int, dof_id_type>, dof_id_type>>
      node_index_map;

  if (!_neighbors_cached)
  {
    for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
    {
      System * to_sys = find_sys(*_to_es[i_to], _to_var_name);
      unsigned int sys_num = to_sys->number();
      unsigned int var_num = to_sys->variable_number(_to_var_name);
      MeshBase * to_mesh = &_to_meshes[i_to]->getMesh();
      const auto to_global_num =
          _current_direction == FROM_MULTIAPP ? 0 : _to_local2global_map[i_to];
      const auto & to_transform = *_to_transforms[to_global_num];
      auto & fe_type = to_sys->variable_type(var_num);
      bool is_constant = fe_type.order == CONSTANT;
      bool is_to_nodal = fe_type.family == LAGRANGE;

      // We support L2_LAGRANGE elemental variable with the first order
      if (fe_type.order > FIRST && !is_to_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      if (!is_to_nodal && isParamValid("target_boundary"))
        paramWarning("target_boundary",
                     "Setting a target boundary is only valid for receiving "
                     "variables of the LAGRANGE basis");

      if (is_to_nodal)
      {
        const std::vector<Node *> & target_local_nodes = getTargetLocalNodes(i_to);

        // For error checking: keep track of all target_local_nodes
        // which are successfully mapped to at least one domain where
        // the nearest neighbor might be found.
        std::set<Node *> local_nodes_found;

        for (const auto & node : target_local_nodes)
        {
          // Skip this node if the variable has no dofs at it.
          if (node->n_dofs(sys_num, var_num) < 1)
            continue;

          const auto transformed_node = to_transform(*node);

          // Find which bboxes might have the nearest node to this point.
          Real nearest_max_distance = std::numeric_limits<Real>::max();
          for (const auto & bbox : bboxes)
          {
            Real distance = bboxMaxDistance(transformed_node, bbox);
            if (distance < nearest_max_distance)
              nearest_max_distance = distance;
          }

          unsigned int from0 = 0;
          for (processor_id_type i_proc = 0; i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc]; i_from++)
            {
              Real distance = bboxMinDistance(transformed_node, bboxes[i_from]);

              if (distance <= nearest_max_distance ||
                  bboxes[i_from].contains_point(transformed_node))
              {
                std::pair<unsigned int, dof_id_type> key(i_to, node->id());
                // Record a local ID for each quadrature point
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(transformed_node);
                local_nodes_found.insert(node);
              }
            }
          }
        }

        // By the time we get to here, we should have found at least
        // one candidate BoundingBox for every node in the
        // target_local_nodes array that has dofs for the current
        // variable in the current System.
        for (const auto & node : target_local_nodes)
          if (node->n_dofs(sys_num, var_num) && !local_nodes_found.count(node))
            mooseError("In ",
                       name(),
                       ": No candidate BoundingBoxes found for node ",
                       node->id(),
                       " at position ",
                       to_transform(*node));
      }
      else // Elemental
      {
        // For error checking: keep track of all local elements
        // which are successfully mapped to at least one domain where
        // the nearest neighbor might be found.
        std::set<Elem *> local_elems_found;
        std::vector<Point> points;
        std::vector<dof_id_type> point_ids;
        for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
        {
          // Skip this element if the variable has no dofs at it.
          if (elem->n_dofs(sys_num, var_num) < 1)
            continue;

          points.clear();
          point_ids.clear();
          // For constant monomial, we take the centroid of element
          if (is_constant)
          {
            points.push_back(to_transform(elem->vertex_average()));
            point_ids.push_back(elem->id());
          }

          // For L2_LAGRANGE, we take all the nodes of element
          else
            for (auto & node : elem->node_ref_range())
            {
              points.push_back(to_transform(node));
              point_ids.push_back(node.id());
            }

          unsigned int offset = 0;
          for (auto & point : points)
          {
            // Find which bboxes might have the nearest node to this point.
            Real nearest_max_distance = std::numeric_limits<Real>::max();
            for (const auto & bbox : bboxes)
            {
              Real distance = bboxMaxDistance(point, bbox);
              if (distance < nearest_max_distance)
                nearest_max_distance = distance;
            }

            unsigned int from0 = 0;
            for (processor_id_type i_proc = 0; i_proc < n_processors();
                 from0 += froms_per_proc[i_proc], i_proc++)
            {
              for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc]; i_from++)
              {
                Real distance = bboxMinDistance(point, bboxes[i_from]);
                if (distance <= nearest_max_distance || bboxes[i_from].contains_point(point))
                {
                  std::pair<unsigned int, dof_id_type> key(
                      i_to,
                      point_ids[offset]); // Create an unique ID
                  // If this point already exist, we skip it
                  if (node_index_map[i_proc].find(key) != node_index_map[i_proc].end())
                    continue;
                  node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                  outgoing_qps[i_proc].push_back(point);
                  local_elems_found.insert(elem);
                } // if distance
              }   // for i_from
            }     // for i_proc
            offset++;
          } // point
        }   // for elem

        // Verify that we found at least one candidate bounding
        // box for each local element with dofs for the current
        // variable in the current System.
        for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
          if (elem->n_dofs(sys_num, var_num) && !local_elems_found.count(elem))
            mooseError("In ",
                       name(),
                       ": No candidate BoundingBoxes found for Elem ",
                       elem->id(),
                       ", centroid = ",
                       to_transform(elem->vertex_average()));
      }
    }
  }

  ////////////////////
  // Send local node/centroid positions off to the other processors and take
  // care of points sent to this processor.  We'll need to check the points
  // against all of the "from" domains that this processor owns.  For each
  // point, we'll find the nearest node, then we'll send the value at that node
  // and the distance between the node and the point back to the processor that
  // requested that point.
  ////////////////////

  std::map<processor_id_type, std::vector<Real>> incoming_evals;

  // Create these here so that they live the entire life of this function
  // and are NOT reused per processor.
  std::map<processor_id_type, std::vector<Real>> processor_outgoing_evals;

  if (!_neighbors_cached)
  {
    // Build an array of pointers to all of this processor's local entities (nodes or
    // elements).  We need to do this to avoid the expense of using LibMesh iterators.
    // This step also takes care of limiting the search to boundary nodes, if
    // applicable.
    std::vector<std::vector<std::pair<Point, DofObject *>>> local_entities(
        froms_per_proc[processor_id()]);

    std::vector<std::vector<unsigned int>> local_comps(froms_per_proc[processor_id()]);

    // Local array of all from Variable references
    std::vector<std::reference_wrapper<MooseVariableFEBase>> _from_vars;

    for (unsigned int i_local_from = 0; i_local_from < froms_per_proc[processor_id()];
         i_local_from++)
    {
      MooseVariableFEBase & from_var = _from_problems[i_local_from]->getVariable(
          0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      auto & from_fe_type = from_var.feType();
      bool is_constant = from_fe_type.order == CONSTANT;
      bool is_to_nodal = from_fe_type.family == LAGRANGE;

      // We support L2_LAGRANGE elemental variable with the first order
      if (from_fe_type.order > FIRST && !is_to_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      _from_vars.emplace_back(from_var);
      getLocalEntitiesAndComponents(_from_meshes[i_local_from],
                                    local_entities[i_local_from],
                                    local_comps[i_local_from],
                                    is_to_nodal,
                                    is_constant);
    }

    // Quadrature points I will receive from remote processors
    std::map<processor_id_type, std::vector<Point>> incoming_qps;
    auto qps_action_functor = [&incoming_qps](processor_id_type pid, const std::vector<Point> & qps)
    {
      // Quadrature points from processor 'pid'
      auto & incoming_qps_from_pid = incoming_qps[pid];
      // Store data for late use
      incoming_qps_from_pid.reserve(incoming_qps_from_pid.size() + qps.size());
      std::copy(qps.begin(), qps.end(), std::back_inserter(incoming_qps_from_pid));
    };

    Parallel::push_parallel_vector_data(comm(), outgoing_qps, qps_action_functor);

    for (auto & qps : incoming_qps)
    {
      const processor_id_type pid = qps.first;

      if (_fixed_meshes)
      {
        auto & froms = _cached_froms[pid];
        froms.resize(qps.second.size());
        std::fill(froms.begin(), froms.end(), libMesh::invalid_uint);

        auto & dof_ids = _cached_dof_ids[pid];
        dof_ids.resize(qps.second.size());
        std::fill(dof_ids.begin(), dof_ids.end(), DofObject::invalid_id);
      }

      std::vector<Real> & outgoing_evals = processor_outgoing_evals[pid];
      // Resize this vector to two times the size of the incoming_qps
      // vector because we are going to store both the value from the nearest
      // local node *and* the distance between the incoming_qp and that node
      // for later comparison purposes.
      outgoing_evals.resize(2 * qps.second.size());

      for (std::size_t qp = 0; qp < qps.second.size(); qp++)
      {
        const Point & qpt = qps.second[qp];
        outgoing_evals[2 * qp] = std::numeric_limits<Real>::max();
        for (unsigned int i_local_from = 0; i_local_from < froms_per_proc[processor_id()];
             i_local_from++)
        {
          MooseVariableFEBase & from_var = _from_vars[i_local_from];
          System & from_sys = from_var.sys().system();
          unsigned int from_sys_num = from_sys.number();
          unsigned int from_var_num = from_sys.variable_number(from_var.name());
          const auto from_global_num =
              _current_direction == TO_MULTIAPP ? 0 : _from_local2global_map[i_local_from];
          const auto & from_transform = *_from_transforms[from_global_num];

          for (unsigned int i_node = 0; i_node < local_entities[i_local_from].size(); i_node++)
          {
            // Compute distance between the current incoming_qp to local node i_node. No need to
            // transform the 'to' because we already did it
            Real current_distance =
                (qpt - from_transform(local_entities[i_local_from][i_node].first)).norm();

            // If an incoming_qp is equally close to two or more local nodes, then
            // the first one we test will "win", even though any of the others could
            // also potentially be chosen instead... there's no way to decide among
            // the set of all equidistant points.
            //
            // outgoing_evals[2 * qp] is the current closest distance between a local point and
            // the incoming_qp.
            if (current_distance < outgoing_evals[2 * qp])
            {
              // Assuming LAGRANGE!
              if (local_entities[i_local_from][i_node].second->n_dofs(from_sys_num, from_var_num) >
                  0)
              {
                dof_id_type from_dof = local_entities[i_local_from][i_node].second->dof_number(
                    from_sys_num, from_var_num, local_comps[i_local_from][i_node]);

                // The indexing of the outgoing_evals vector looks
                // like [(distance, value), (distance, value), ...]
                // for each incoming_qp. We only keep the value from
                // the node with the smallest distance to the
                // incoming_qp, and then we compare across all
                // processors later and pick the closest one.
                outgoing_evals[2 * qp] = current_distance;
                outgoing_evals[2 * qp + 1] = (*from_sys.solution)(from_dof);

                if (_fixed_meshes)
                {
                  // Cache the nearest nodes.
                  _cached_froms[pid][qp] = i_local_from;
                  _cached_dof_ids[pid][qp] = from_dof;
                }
              }
            }
          }
        }
      }
    }
  }

  else // We've cached the nearest nodes.
  {
    for (auto & problem_from : _cached_froms)
    {
      const processor_id_type pid = problem_from.first;
      std::vector<Real> & outgoing_evals = processor_outgoing_evals[pid];
      outgoing_evals.resize(problem_from.second.size());

      for (unsigned int qp = 0; qp < outgoing_evals.size(); qp++)
      {
        const auto from_problem = problem_from.second[qp];
        if (from_problem == libMesh::invalid_uint)
        {
          mooseAssert(_cached_dof_ids[pid][qp] == DofObject::invalid_id,
                      "The state of the from problem and dof id should match.");
          continue;
        }

        MooseVariableFEBase & from_var =
            _from_problems[from_problem]->getVariable(0,
                                                      _from_var_name,
                                                      Moose::VarKindType::VAR_ANY,
                                                      Moose::VarFieldType::VAR_FIELD_STANDARD);
        System & from_sys = from_var.sys().system();
        dof_id_type from_dof = _cached_dof_ids[pid][qp];
        outgoing_evals[qp] = (*from_sys.solution)(from_dof);
      }
    }
  }

  auto evals_action_functor =
      [&incoming_evals](processor_id_type pid, const std::vector<Real> & evals)
  {
    // evals for processor 'pid'
    auto & incoming_evals_for_pid = incoming_evals[pid];
    // Copy evals for late use
    incoming_evals_for_pid.reserve(incoming_evals_for_pid.size() + evals.size());
    std::copy(evals.begin(), evals.end(), std::back_inserter(incoming_evals_for_pid));
  };

  Parallel::push_parallel_vector_data(comm(), processor_outgoing_evals, evals_action_functor);

  ////////////////////
  // Gather all of the evaluations, find the nearest one for each node/element,
  // and apply the values.
  ////////////////////

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    // Loop over the master nodes and set the value of the variable
    System * to_sys = find_sys(*_to_es[i_to], _to_var_name);

    unsigned int sys_num = to_sys->number();
    unsigned int var_num = to_sys->variable_number(_to_var_name);

    NumericVector<Real> * solution = nullptr;
    switch (_current_direction)
    {
      case TO_MULTIAPP:
        solution = &getTransferVector(i_to, _to_var_name);
        break;
      case FROM_MULTIAPP:
        solution = to_sys->solution.get();
        break;
      default:
        mooseError("Unknown direction");
    }

    const MeshBase & to_mesh = _to_meshes[i_to]->getMesh();

    auto & fe_type = to_sys->variable_type(var_num);
    bool is_constant = fe_type.order == CONSTANT;
    bool is_to_nodal = fe_type.family == LAGRANGE;

    // We support L2_LAGRANGE elemental variable with the first order
    if (fe_type.order > FIRST && !is_to_nodal)
      mooseError("We don't currently support second order or higher elemental variable ");

    if (is_to_nodal)
    {
      const std::vector<Node *> & target_local_nodes = getTargetLocalNodes(i_to);

      for (const auto & node : target_local_nodes)
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // If nothing is in the node_index_map for a given local node,
        // it will get the value 0.
        Real best_val = 0;
        if (!_neighbors_cached)
        {
          // Search through all the incoming evaluation points from
          // different processors for the one with the closest
          // point. If there are multiple values from other processors
          // which are equidistant, the first one we check will "win".
          Real min_dist = std::numeric_limits<Real>::max();
          for (auto & evals : incoming_evals)
          {
            // processor Id
            const processor_id_type pid = evals.first;
            std::pair<unsigned int, dof_id_type> key(i_to, node->id());
            if (node_index_map[pid].find(key) == node_index_map[pid].end())
              continue;
            unsigned int qp_ind = node_index_map[pid][key];
            // Distances
            if (evals.second[2 * qp_ind] >= min_dist)
              continue;

            // If we made it here, we are going to set a new value and
            // distance because we found one that was closer.
            min_dist = evals.second[2 * qp_ind];
            best_val = evals.second[2 * qp_ind + 1];

            if (_fixed_meshes)
            {
              // Cache these indices.
              _cached_from_inds[std::make_pair(i_to, node->id())] = pid;
              _cached_qp_inds[std::make_pair(i_to, node->id())] = qp_ind;
            }
          }
        }

        else
        {
          best_val = incoming_evals[_cached_from_inds[std::make_pair(i_to, node->id())]]
                                   [_cached_qp_inds[std::make_pair(i_to, node->id())]];
        }

        dof_id_type dof = node->dof_number(sys_num, var_num, 0);
        solution->set(dof, best_val);
      }
    }
    else // Elemental
    {
      std::vector<Point> points;
      std::vector<dof_id_type> point_ids;
      for (auto & elem : to_mesh.active_local_element_ptr_range())
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
          for (auto & node : elem->node_ref_range())
          {
            points.push_back(node);
            point_ids.push_back(node.id());
          }

        unsigned int n_comp = elem->n_comp(sys_num, var_num);
        // We assume each point corresponds to one component of elemental variable
        if (points.size() != n_comp)
          mooseError(" Number of points ",
                     points.size(),
                     " does not equal to number of variable components ",
                     n_comp);

        for (MooseIndex(points) offset = 0; offset < points.size(); offset++)
        {
          dof_id_type point_id = point_ids[offset];
          Real best_val = 0;
          if (!_neighbors_cached)
          {
            Real min_dist = std::numeric_limits<Real>::max();
            for (auto & evals : incoming_evals)
            {
              const processor_id_type pid = evals.first;

              std::pair<unsigned int, dof_id_type> key(i_to, point_id);
              if (node_index_map[pid].find(key) == node_index_map[pid].end())
                continue;

              unsigned int qp_ind = node_index_map[pid][key];
              if (evals.second[2 * qp_ind] >= min_dist)
                continue;

              min_dist = evals.second[2 * qp_ind];
              best_val = evals.second[2 * qp_ind + 1];

              if (_fixed_meshes)
              {
                // Cache these indices.
                _cached_from_inds[std::make_pair(i_to, point_id)] = pid;
                _cached_qp_inds[std::make_pair(i_to, point_id)] = qp_ind;
              } // if _fixed_meshes
            }   // i_from
          }     //
          else
          {
            best_val = incoming_evals[_cached_from_inds[std::make_pair(i_to, point_id)]]
                                     [_cached_qp_inds[std::make_pair(i_to, point_id)]];
          }
          dof_id_type dof = elem->dof_number(sys_num, var_num, offset);
          solution->set(dof, best_val);
        } // for offset
      }
    }
    solution->close();
    to_sys->update();
  }

  if (_fixed_meshes)
    _neighbors_cached = true;

  postExecute();
}

Real
MultiAppNearestNodeTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox)
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
MultiAppNearestNodeTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox)
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

void
MultiAppNearestNodeTransfer::getLocalEntitiesAndComponents(
    MooseMesh * mesh,
    std::vector<std::pair<Point, DofObject *>> & local_entities,
    std::vector<unsigned int> & local_comps,
    bool is_nodal,
    bool is_constant)
{
  mooseAssert(mesh, "mesh should not be a nullptr");
  mooseAssert(local_entities.empty(), "local_entities should be empty");
  MeshBase & mesh_base = mesh->getMesh();

  if (isParamValid("source_boundary"))
  {
    const auto & sb = getParam<BoundaryName>("source_boundary");
    BoundaryID src_bnd_id = mesh->getBoundaryID(sb);
    if (!MooseMeshUtils::hasBoundaryName(mesh_base, sb))
      paramError("source_boundary", "The boundary '", sb, "' was not found in the mesh");

    if (is_nodal)
    {
      const ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
      for (const auto & bnode : bnd_nodes)
      {
        unsigned int comp = 0;
        if (bnode->_bnd_id == src_bnd_id &&
            bnode->_node->processor_id() == mesh_base.processor_id())
        {
          local_entities.emplace_back(*bnode->_node, bnode->_node);
          local_comps.push_back(comp++);
        }
      }
    }
    else
    {
      const ConstBndElemRange & bnd_elems = *mesh->getBoundaryElementRange();
      for (const auto & belem : bnd_elems)
      {
        unsigned int comp = 0;
        if (belem->_bnd_id == src_bnd_id &&
            belem->_elem->processor_id() == mesh_base.processor_id())
        {
          // CONSTANT Monomial
          if (is_constant)
          {
            local_entities.emplace_back(belem->_elem->vertex_average(), belem->_elem);
            local_comps.push_back(comp++);
          }
          // L2_LAGRANGE
          else
          {
            for (auto & node : belem->_elem->node_ref_range())
            {
              local_entities.emplace_back(node, belem->_elem);
              local_comps.push_back(comp++);
            }
          }
        }
      }
    }
  }
  else
  {
    if (is_nodal)
    {
      local_entities.reserve(mesh_base.n_local_nodes());
      for (auto & node : mesh_base.local_node_ptr_range())
      {
        unsigned int comp = 0;
        local_entities.emplace_back(*node, node);
        local_comps.push_back(comp++);
      }
    }
    else
    {
      local_entities.reserve(mesh_base.n_local_elem());
      for (auto & elem : mesh_base.active_local_element_ptr_range())
      {
        unsigned int comp = 0;
        // CONSTANT Monomial
        if (is_constant)
        {
          local_entities.emplace_back(elem->vertex_average(), elem);
          local_comps.push_back(comp++);
        }
        // L2_LAGRANGE
        else
        {
          for (auto & node : elem->node_ref_range())
          {
            local_entities.emplace_back(node, elem);
            local_comps.push_back(comp++);
          }
        }
      }
    }
  }
}

const std::vector<Node *> &
MultiAppNearestNodeTransfer::getTargetLocalNodes(const unsigned int to_problem_id)
{
  _target_local_nodes.clear();
  MeshBase & to_mesh = _to_meshes[to_problem_id]->getMesh();

  if (isParamValid("target_boundary"))
  {
    const std::vector<BoundaryName> & target_boundaries =
        getParam<std::vector<BoundaryName>>("target_boundary");
    for (const auto & b : target_boundaries)
      if (!MooseMeshUtils::hasBoundaryName(to_mesh, b))
        paramError("target_boundary", "The boundary '", b, "' was not found in the mesh");

    ConstBndNodeRange & bnd_nodes = *(_to_meshes[to_problem_id])->getBoundaryNodeRange();

    for (const auto & t : target_boundaries)
    {
      BoundaryID target_bnd_id = _to_meshes[to_problem_id]->getBoundaryID(t);

      for (const auto & bnode : bnd_nodes)
        if (bnode->_bnd_id == target_bnd_id &&
            bnode->_node->processor_id() == _to_meshes[to_problem_id]->processor_id())
          _target_local_nodes.push_back(bnode->_node);
    }
  }
  else
  {
    _target_local_nodes.resize(to_mesh.n_local_nodes());
    unsigned int i = 0;
    for (auto & node : to_mesh.local_node_ptr_range())
      _target_local_nodes[i++] = node;
  }

  return _target_local_nodes;
}
