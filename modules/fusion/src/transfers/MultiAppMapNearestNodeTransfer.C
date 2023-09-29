//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppMapNearestNodeTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"

#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/dof_object.h"

// TIMPI includes
#include "timpi/parallel_sync.h"

registerMooseObject("FusionApp", MultiAppMapNearestNodeTransfer);

InputParameters
MultiAppMapNearestNodeTransfer::validParams()
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

  params.addParam<Real>(
      "bbox_extend_factor", 0, "Expand bounding box by a factor in all the directions");
  return params;
}

MultiAppMapNearestNodeTransfer::MultiAppMapNearestNodeTransfer(const InputParameters & parameters)
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
        declareRestartableData<std::map<dof_id_type, unsigned int>>("cached_from_ids")),
    _cached_qp_inds(declareRestartableData<std::map<dof_id_type, unsigned int>>("cached_qp_inds")),
    _bbox_extend_factor(getParam<Real>("bbox_extend_factor"))
{
  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only");

  if (_from_var_names.size() != 1)
    paramError("source_variable", " Support single from-variable only");
}

void
MultiAppMapNearestNodeTransfer::execute()
{
  _console << "Beginning MapNearestNodeTransfer " << name() << std::endl;

  getAppInfo();

  // All boundaries are required by subapps
  // each subapp corresponds to one boundary in master
  std::vector<BoundaryID> vec_bdry_ids;
  auto & master_problem = _multi_app->problemBase();
  auto & master_mesh = master_problem.mesh();
  for (unsigned int i_app = 0; i_app < _multi_app->numGlobalApps(); i_app++)
  {
    if (!_multi_app->hasLocalApp(i_app))
    {
      // We do not really need this but it is
      // convenient to have it for MPI communication
      vec_bdry_ids.push_back(Moose::ANY_BOUNDARY_ID);
      continue;
    }
    auto & fe_problem = _multi_app->appProblemBase(i_app);
    // If this subapp has set boundary name which it wants to associate with
    if (fe_problem.getMasterBoundaryName() != "")
    {
      auto & bdryname = fe_problem.getMasterBoundaryName();
      // Get Boundary ID out of it
      vec_bdry_ids.push_back(master_mesh.getBoundaryID(bdryname));
    }
    else
      vec_bdry_ids.push_back(Moose::ANY_BOUNDARY_ID);
  }

  _communicator.allgather(vec_bdry_ids);

  _subapp_id_to_bdryid.clear();
  dof_id_type index = 0;
  for (auto bdry_id : vec_bdry_ids)
  {
    // We did "allgather", and everyone should a global picture
    // each subapp (local and nonlocal) should its master boundary ID
    if (bdry_id != Moose::ANY_BOUNDARY_ID)
      _subapp_id_to_bdryid[index % _multi_app->numGlobalApps()] = bdry_id;

    index++;
  }

  // Boundary IDs owned by current processor of the master app
  std::set<BoundaryID> curr_bdry_ids;
  getBoundaryIDsForCurrProcessor(master_mesh, curr_bdry_ids);

  // Fill in 'Moose::ANY_BOUNDARY_ID' for the boundaries not owned
  // by the current processor
  std::vector<BoundaryID> all_bdry_ids;

  for (auto bdry_id : _subapp_id_to_bdryid)
  {
    auto it = curr_bdry_ids.find(bdry_id.second);

    if (it != curr_bdry_ids.end())
      all_bdry_ids.push_back(bdry_id.second);
    else
      all_bdry_ids.push_back(Moose::ANY_BOUNDARY_ID);
  }

  _communicator.allgather(all_bdry_ids);

  // Build a map between pid and boundary IDs
  std::unordered_map<processor_id_type, std::set<BoundaryID>> pid_bdry_ids;

  index = 0;
  for (auto bdry_id : all_bdry_ids)
  {
    if (bdry_id != Moose::ANY_BOUNDARY_ID)
      pid_bdry_ids[index / _multi_app->numGlobalApps()].insert(bdry_id);

    index++;
  }

  // Get the bounding boxes for the "from" domains.
  std::vector<BoundingBox> bboxes;
  if (isParamValid("source_boundary"))
  {
    bboxes = MultiAppTransfer::getFromBoundingBoxes(
        _from_meshes[0]->getBoundaryID(getParam<BoundaryName>("source_boundary")));
  }
  // Use boundaries as source
  else if (_current_direction == TO_MULTIAPP && _subapp_id_to_bdryid.size())
  {
    std::set<BoundaryID> bdry_ids;
    for (auto it : _subapp_id_to_bdryid)
      bdry_ids.insert(it.second);

    bboxes = getFromBoundingBoxes(bdry_ids);
  }
  else
    bboxes = MultiAppTransfer::getFromBoundingBoxes();

  // Expand bounding boxes along all the directions by the same length
  // Non-zero values of this member may be necessary because the nearest
  // bounding box does not necessarily give you the closest node/element.
  // It will depend on the partition and geometry. A node/element will more
  // likely find its nearest source element/node by extending
  // bounding boxes. If each of the bounding boxes covers the entire domain,
  // a node/element will be able to find its nearest source element/node for sure,
  // but at the same time, more communication will be involved and can be expensive.
  for (auto & box : bboxes)
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
    box.second += width * _bbox_extend_factor;
    box.first -= width * _bbox_extend_factor;
  }

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  std::vector<unsigned int> local2global_map(_from_local2global_map);

  // local to global maps for all processors
  _communicator.allgather(local2global_map);

  ////////////////////
  // For every point in the local "to" domain, figure out which "from" domains
  // might contain it's nearest neighbor, and send that point to the processors
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
  std::map<processor_id_type, std::vector<std::pair<dof_id_type, Point>>> outgoing_qps;
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
      auto & fe_type = to_sys->variable_type(var_num);
      bool is_constant = fe_type.order == CONSTANT;
      bool is_to_nodal = fe_type.family == LAGRANGE;

      // We support L2_LAGRANGE elemental variable with the first order
      if (fe_type.order > FIRST && !is_to_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      if (is_to_nodal)
      {
        const auto & target_local_nodes = getTargetLocalNodes(i_to);

        // For error checking: keep track of all target_local_nodes
        // which are successfully mapped to at least one domain where
        // the nearest neighbor might be found.
        std::set<Node *> local_nodes_found;

        for (const auto & node : target_local_nodes)
        {
          // Skip this node if the variable has no dofs at it.
          if (node.first->n_dofs(sys_num, var_num) < 1)
            continue;

          // Find which bboxes might have the nearest node to this point.
          Real nearest_max_distance = std::numeric_limits<Real>::max();
          for (const auto & bbox : bboxes)
          {
            Real distance = bboxMaxDistance(*node.first, bbox);
            if (distance < nearest_max_distance)
              nearest_max_distance = distance;
          }

          unsigned int from0 = 0;
          for (processor_id_type i_proc = 0; i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc]; i_from++)
            {

              Real distance = bboxMinDistance(*node.first, bboxes[i_from]);

              if (_current_direction == FROM_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
              {
                // If current node boundary match subapp id
                if (node.second == _subapp_id_to_bdryid[local2global_map[i_from]])
                {
                  std::pair<unsigned int, dof_id_type> key(i_to, node.first->id());
                  // Record a local ID for each quadrature point
                  node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                  outgoing_qps[i_proc].push_back(
                      std::make_pair(local2global_map[i_from], *node.first + _to_positions[i_to]));
                  local_nodes_found.insert(node.first);
                }
              }
              else if (_current_direction == TO_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
              {
                // Find right boundaryid for i_to subapp
                auto from_bdry_id = _subapp_id_to_bdryid[_to_local2global_map[i_to]];
                auto & i_from_bdry_ids = pid_bdry_ids[i_proc];
                auto it = i_from_bdry_ids.find(from_bdry_id);
                if (it != i_from_bdry_ids.end())
                {
                  std::pair<unsigned int, dof_id_type> key(i_to, node.first->id());
                  // Record a local ID for each quadrature point
                  node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                  outgoing_qps[i_proc].push_back(std::make_pair(_to_local2global_map[i_to],
                                                                *node.first + _to_positions[i_to]));
                  local_nodes_found.insert(node.first);
                }
              }
              else if (distance <= nearest_max_distance ||
                       bboxes[i_from].contains_point(*node.first))
              {
                std::pair<unsigned int, dof_id_type> key(i_to, node.first->id());
                // Record a local ID for each quadrature point
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(
                    std::make_pair(_to_local2global_map[i_to], *node.first + _to_positions[i_to]));
                local_nodes_found.insert(node.first);
              }
            }
          }
        }

        // By the time we get to here, we should have found at least
        // one candidate BoundingBox for every node in the
        // target_local_nodes array that has dofs for the current
        // variable in the current System.
        for (const auto & node : target_local_nodes)
          if (node.first->n_dofs(sys_num, var_num) && !local_nodes_found.count(node.first))
            mooseError("In ",
                       name(),
                       ": No candidate BoundingBoxes found for node ",
                       node.first->id(),
                       " at position ",
                       *node.first);
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
            points.push_back(elem->vertex_average());
            point_ids.push_back(elem->id());
          }

          // For L2_LAGRANGE, we take all the nodes of element
          else
            for (auto & node : elem->node_ref_range())
            {
              points.push_back(node);
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
                if (_current_direction == TO_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
                {
                  auto from_bdry_id = _subapp_id_to_bdryid[_to_local2global_map[i_to]];
                  auto & i_from_bdry_ids = pid_bdry_ids[i_proc];
                  auto it = i_from_bdry_ids.find(from_bdry_id);
                  if (it != i_from_bdry_ids.end())
                  {
                    std::pair<unsigned int, dof_id_type> key(
                        i_to,
                        point_ids[offset]); // Create an unique ID
                    // If this point already exist, we skip it
                    if (node_index_map[i_proc].find(key) != node_index_map[i_proc].end())
                      continue;

                    node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                    outgoing_qps[i_proc].push_back(
                        std::make_pair(_to_local2global_map[i_to], point + _to_positions[i_to]));
                    local_elems_found.insert(elem);
                  }
                }
                else if (distance <= nearest_max_distance || bboxes[i_from].contains_point(point))
                {
                  std::pair<unsigned int, dof_id_type> key(
                      i_to,
                      point_ids[offset]); // Create an unique ID
                  // If this point already exist, we skip it
                  if (node_index_map[i_proc].find(key) != node_index_map[i_proc].end())
                    continue;
                  node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                  outgoing_qps[i_proc].push_back(
                      std::make_pair(_to_local2global_map[i_to], point + _to_positions[i_to]));
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
                       elem->vertex_average());
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
    std::vector<std::vector<std::tuple<Point, DofObject *, BoundaryID>>> local_entities(
        froms_per_proc[processor_id()]);

    std::vector<std::vector<unsigned int>> local_comps(froms_per_proc[processor_id()]);

    // Local array of all from Variable references
    std::vector<std::reference_wrapper<MooseVariableFEBase>> _from_vars;

    for (unsigned int i = 0; i < froms_per_proc[processor_id()]; i++)
    {
      MooseVariableFEBase & from_var = _from_problems[i]->getVariable(
          0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      auto & from_fe_type = from_var.feType();
      bool is_constant = from_fe_type.order == CONSTANT;
      bool is_to_nodal = from_fe_type.family == LAGRANGE;

      // We support L2_LAGRANGE elemental variable with the first order
      if (from_fe_type.order > FIRST && !is_to_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      _from_vars.emplace_back(from_var);
      getLocalEntitiesAndComponents(
          _from_meshes[i], local_entities[i], local_comps[i], is_to_nodal, is_constant);
    }

    // Quadrature points I will receive from remote processors
    std::map<processor_id_type, std::vector<std::pair<dof_id_type, Point>>> incoming_qps;
    auto qps_action_functor =
        [&incoming_qps](processor_id_type pid,
                        const std::vector<std::pair<dof_id_type, Point>> & qps)
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
        _cached_froms[pid].resize(qps.second.size());
        _cached_dof_ids[pid].resize(qps.second.size());
      }

      std::vector<Real> & outgoing_evals = processor_outgoing_evals[pid];
      // Resize this vector to two times the size of the incoming_qps
      // vector because we are going to store both the value from the nearest
      // local node *and* the distance between the incoming_qp and that node
      // for later comparison purposes.
      outgoing_evals.resize(2 * qps.second.size());

      for (std::size_t qp = 0; qp < qps.second.size(); qp++)
      {
        const Point & qpt = qps.second[qp].second;
        auto subapp_id = qps.second[qp].first;
        outgoing_evals[2 * qp] = std::numeric_limits<Real>::max();
        for (unsigned int i_local_from = 0; i_local_from < froms_per_proc[processor_id()];
             i_local_from++)
        {
          MooseVariableFEBase & from_var = _from_vars[i_local_from];
          System & from_sys = from_var.sys().system();
          unsigned int from_sys_num = from_sys.number();
          unsigned int from_var_num = from_sys.variable_number(from_var.name());

          for (unsigned int i_node = 0; i_node < local_entities[i_local_from].size(); i_node++)
          {
            auto & local_point = std::get<0>(local_entities[i_local_from][i_node]);
            // Compute distance between the current incoming_qp to local node i_node.
            Real current_distance = (qpt - local_point - _from_positions[i_local_from]).norm();

            auto & local_bdry_id = std::get<2>(local_entities[i_local_from][i_node]);
            if (_current_direction == TO_MULTIAPP &&
                _subapp_id_to_bdryid[subapp_id] == local_bdry_id)
              current_distance -= 1000;

            if (_current_direction == FROM_MULTIAPP &&
                subapp_id == _from_local2global_map[i_local_from])
              current_distance -= 1000;

            // If an incoming_qp is equally close to two or more local nodes, then
            // the first one we test will "win", even though any of the others could
            // also potentially be chosen instead... there's no way to decide among
            // the set of all equidistant points.
            //
            // outgoing_evals[2 * qp] is the current closest distance between a local point and
            // the incoming_qp.
            if (current_distance < outgoing_evals[2 * qp])
            {
              auto & local_dof_object = std::get<1>(local_entities[i_local_from][i_node]);
              // Assuming LAGRANGE!
              if (local_dof_object->n_dofs(from_sys_num, from_var_num) > 0)
              {
                dof_id_type from_dof = local_dof_object->dof_number(
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
        MooseVariableFEBase & from_var = _from_problems[problem_from.second[qp]]->getVariable(
            0,
            _from_var_name,
            Moose::VarKindType::VAR_ANY,
            Moose::VarFieldType::VAR_FIELD_STANDARD);
        System & from_sys = from_var.sys().system();
        dof_id_type from_dof = _cached_dof_ids[pid][qp];
        // outgoing_evals[qp] = (*from_sys.solution)(_cached_dof_ids[pid][qp]);
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
      const std::vector<std::pair<Node *, BoundaryID>> & target_local_nodes =
          getTargetLocalNodes(i_to);

      for (const auto & node : target_local_nodes)
      {
        // Skip this node if the variable has no dofs at it.
        if (node.first->n_dofs(sys_num, var_num) < 1)
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
            std::pair<unsigned int, dof_id_type> key(i_to, node.first->id());
            if (node_index_map[pid].find(key) == node_index_map[pid].end())
              continue;
            unsigned int qp_ind = node_index_map[pid][key];
            // Distances
            if (evals.second[2 * qp_ind] >= min_dist)
              continue;

            // If we made it here, we are going set a new value and
            // distance because we found one that was closer.
            min_dist = evals.second[2 * qp_ind];
            best_val = evals.second[2 * qp_ind + 1];

            if (_fixed_meshes)
            {
              // Cache these indices.
              _cached_from_inds[node.first->id()] = pid;
              _cached_qp_inds[node.first->id()] = qp_ind;
            }
          }
        }

        else
        {
          best_val = incoming_evals[_cached_from_inds[node.first->id()]]
                                   [_cached_qp_inds[node.first->id()]];
        }

        dof_id_type dof = node.first->dof_number(sys_num, var_num, 0);
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
                _cached_from_inds[point_id] = pid;
                _cached_qp_inds[point_id] = qp_ind;
              } // if _fixed_meshes
            }   // i_from
          }     //
          else
          {
            best_val = incoming_evals[_cached_from_inds[point_id]][_cached_qp_inds[point_id]];
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

  _console << "Finished MapNearestNodeTransfer " << name() << std::endl;

  postExecute();
}

Real
MultiAppMapNearestNodeTransfer::bboxMaxDistance(const Point & p, const BoundingBox & bbox)
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
MultiAppMapNearestNodeTransfer::bboxMinDistance(const Point & p, const BoundingBox & bbox)
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
MultiAppMapNearestNodeTransfer::getLocalEntitiesAndComponents(
    MooseMesh * mesh,
    std::vector<std::tuple<Point, DofObject *, BoundaryID>> & local_entities,
    std::vector<unsigned int> & local_comps,
    bool is_nodal,
    bool is_constant)
{
  mooseAssert(mesh, "mesh should not be a nullptr");
  mooseAssert(local_entities.empty(), "local_entities should be empty");
  const MeshBase & mesh_base = mesh->getMesh();

  if (_current_direction == TO_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
  {

    std::set<BoundaryID> bdry_ids;

    for (auto bdryid : _subapp_id_to_bdryid)
      if (bdryid.second != Moose::ANY_BOUNDARY_ID)
        bdry_ids.insert(bdryid.second);

    if (is_nodal)
    {
      const ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
      for (const auto & bnode : bnd_nodes)
      {
        unsigned int comp = 0;
        auto it = bdry_ids.find(bnode->_bnd_id);
        if (it != bdry_ids.end() && bnode->_node->processor_id() == mesh_base.processor_id())
        {
          local_entities.emplace_back(*bnode->_node, bnode->_node, bnode->_bnd_id);
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
        auto it = bdry_ids.find(belem->_bnd_id);
        if (it != bdry_ids.end() && belem->_elem->processor_id() == mesh_base.processor_id())
        {
          // CONSTANT Monomial
          if (is_constant)
          {
            local_entities.emplace_back(
                belem->_elem->vertex_average(), belem->_elem, belem->_bnd_id);
            local_comps.push_back(comp++);
          }
          // L2_LAGRANGE
          else
          {
            for (auto & node : belem->_elem->node_ref_range())
            {
              local_entities.emplace_back(node, belem->_elem, belem->_bnd_id);
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
        local_entities.emplace_back(*node, node, Moose::ANY_BOUNDARY_ID);
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
          local_entities.emplace_back(elem->vertex_average(), elem, Moose::ANY_BOUNDARY_ID);
          local_comps.push_back(comp++);
        }
        // L2_LAGRANGE
        else
        {
          for (auto & node : elem->node_ref_range())
          {
            local_entities.emplace_back(node, elem, Moose::ANY_BOUNDARY_ID);
            local_comps.push_back(comp++);
          }
        }
      }
    }
  }
}

void
MultiAppMapNearestNodeTransfer::getLocalEntities(
    MooseMesh * mesh,
    std::vector<std::tuple<Point, DofObject *, BoundaryID>> & local_entities,
    bool is_nodal)
{
  mooseAssert(local_entities.empty(), "local_entities should be empty");
  const MeshBase & mesh_base = mesh->getMesh();

  if (_current_direction == TO_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
  {

    std::set<BoundaryID> bdry_ids;

    for (auto bdryid : _subapp_id_to_bdryid)
      if (bdryid.second != Moose::ANY_BOUNDARY_ID)
        bdry_ids.insert(bdryid.second);

    if (is_nodal)
    {
      const ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
      for (const auto & bnode : bnd_nodes)
      {
        auto it = bdry_ids.find(bnode->_bnd_id);

        if (it != bdry_ids.end() && bnode->_node->processor_id() == mesh_base.processor_id())
          local_entities.emplace_back(*bnode->_node, bnode->_node, bnode->_bnd_id);
      }
    }
    else
    {
      const ConstBndElemRange & bnd_elems = *mesh->getBoundaryElementRange();
      for (const auto & belem : bnd_elems)
      {
        auto it = bdry_ids.find(belem->_bnd_id);

        if (it != bdry_ids.end() && belem->_elem->processor_id() == mesh_base.processor_id())
          local_entities.emplace_back(belem->_elem->vertex_average(), belem->_elem, belem->_bnd_id);
      }
    }
  }
  else
  {
    if (is_nodal)
    {
      local_entities.reserve(mesh_base.n_local_nodes());
      for (auto & node : mesh_base.local_node_ptr_range())
        local_entities.emplace_back(*node, node, Moose::ANY_BOUNDARY_ID);
    }
    else
    {
      local_entities.reserve(mesh_base.n_local_elem());
      for (auto & elem : mesh_base.active_local_element_ptr_range())
        local_entities.emplace_back(elem->vertex_average(), elem, Moose::ANY_BOUNDARY_ID);
    }
  }
}

const std::vector<std::pair<Node *, BoundaryID>> &
MultiAppMapNearestNodeTransfer::getTargetLocalNodes(const unsigned int to_problem_id)
{
  _target_local_nodes.clear();
  const MeshBase & to_mesh = _to_meshes[to_problem_id]->getMesh();

  if (_current_direction == FROM_MULTIAPP && _subapp_id_to_bdryid.size() > 0)
  {
    std::set<BoundaryID> bdry_ids;

    for (auto bdryid : _subapp_id_to_bdryid)
      if (bdryid.second != Moose::ANY_BOUNDARY_ID)
        bdry_ids.insert(bdryid.second);

    ConstBndNodeRange & bnd_nodes = *(_to_meshes[to_problem_id])->getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      auto it = bdry_ids.find(bnode->_bnd_id);

      if (it != bdry_ids.end() &&
          bnode->_node->processor_id() == _to_meshes[to_problem_id]->processor_id())
        _target_local_nodes.emplace_back(bnode->_node, bnode->_bnd_id);
    }
  }
  else
  {
    //_target_local_nodes.resize(to_mesh.n_local_nodes());
    // unsigned int i = 0;
    for (auto & node : to_mesh.local_node_ptr_range())
      _target_local_nodes.emplace_back(node, Moose::ANY_BOUNDARY_ID);
  }

  return _target_local_nodes;
}

void
MultiAppMapNearestNodeTransfer::getBoundaryIDsForCurrProcessor(MooseMesh & master_moose_mesh,
                                                               std::set<BoundaryID> & curr_bdry_ids)
{

  std::set<BoundaryID> bdry_ids;

  for (auto bdryid : _subapp_id_to_bdryid)
    if (bdryid.second != Moose::ANY_BOUNDARY_ID)
      bdry_ids.insert(bdryid.second);

  const auto & bnd_nodes = *master_moose_mesh.getBoundaryNodeRange();
  for (const auto & bnode : bnd_nodes)
  {
    auto it = bdry_ids.find(bnode->_bnd_id);

    if (it != bdry_ids.end() && bnode->_node->processor_id() == master_moose_mesh.processor_id())
      curr_bdry_ids.insert(bnode->_bnd_id);
  }
}

std::vector<BoundingBox>
MultiAppMapNearestNodeTransfer::getFromBoundingBoxes(std::set<BoundaryID> bdry_ids)
{
  std::vector<std::pair<Point, Point>> bb_points(_from_meshes.size());
  const Real min_r = std::numeric_limits<Real>::lowest();
  const Real max_r = std::numeric_limits<Real>::max();

  for (unsigned int i = 0; i < _from_meshes.size(); i++)
  {

    Point min(max_r, max_r, max_r);
    Point max(min_r, min_r, min_r);
    bool at_least_one = false;

    // TODO: Factor this into mesh_tools after adding new boundary bounding box routine.
    const ConstBndNodeRange & bnd_nodes = *_from_meshes[i]->getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      auto it = bdry_ids.find(bnode->_bnd_id);
      if (it != bdry_ids.end() && bnode->_node->processor_id() == _from_meshes[i]->processor_id())
      {
        at_least_one = true;
        const auto & node = *bnode->_node;
        for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
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
      bbox.first += _from_positions[i];
      bbox.second += _from_positions[i];
    }

    // Cast the bounding box into a pair of points (so it can be put through
    // MPI communication).
    bb_points[i] = static_cast<std::pair<Point, Point>>(bbox);
  }

  // Serialize the bounding box points.
  _communicator.allgather(bb_points);

  // Recast the points back into bounding boxes and return.
  std::vector<BoundingBox> bboxes(bb_points.size());
  for (unsigned int i = 0; i < bb_points.size(); i++)
    bboxes[i] = static_cast<BoundingBox>(bb_points[i]);

  return bboxes;
}
