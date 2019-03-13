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

#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/dof_object.h"

registerMooseObject("MooseApp", MultiAppNearestNodeTransfer);

template <>
InputParameters
validParams<MultiAppNearestNodeTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<BoundaryName>(
      "source_boundary",
      "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<BoundaryName>(
      "target_boundary",
      "The boundary we are transferring to (if not specified, whole domain is used).");
  params.addParam<bool>("fixed_meshes",
                        false,
                        "Set to true when the meshes are not changing (ie, "
                        "no movement or adaptivity).  This will cache "
                        "nearest node neighbors to greatly speed up the "
                        "transfer.");

  return params;
}

MultiAppNearestNodeTransfer::MultiAppNearestNodeTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _fixed_meshes(getParam<bool>("fixed_meshes")),
    _node_map(declareRestartableData<std::map<dof_id_type, Node *>>("node_map")),
    _distance_map(declareRestartableData<std::map<dof_id_type, Real>>("distance_map")),
    _neighbors_cached(declareRestartableData<bool>("neighbors_cached", false)),
    _cached_froms(declareRestartableData<std::vector<std::vector<unsigned int>>>("cached_froms")),
    _cached_dof_ids(
        declareRestartableData<std::vector<std::vector<dof_id_type>>>("cached_dof_ids")),
    _cached_from_inds(
        declareRestartableData<std::map<dof_id_type, unsigned int>>("cached_from_ids")),
    _cached_qp_inds(declareRestartableData<std::map<dof_id_type, unsigned int>>("cached_qp_inds"))
{
}

void
MultiAppNearestNodeTransfer::initialSetup()
{
  if (_direction == TO_MULTIAPP)
    variableIntegrityCheck(_to_var_name);
  else
    variableIntegrityCheck(_from_var_name);
}

void
MultiAppNearestNodeTransfer::execute()
{
  _console << "Beginning NearestNodeTransfer " << name() << std::endl;

  getAppInfo();

  // Get the bounding boxes for the "from" domains.
  std::vector<BoundingBox> bboxes;
  if (isParamValid("source_boundary"))
    bboxes = getFromBoundingBoxes(
        _from_meshes[0]->getBoundaryID(getParam<BoundaryName>("source_boundary")));
  else
    bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

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
  std::vector<std::vector<Point>> outgoing_qps(n_processors());
  // When we get results back, node_index_map will tell us which results go with
  // which points
  std::vector<std::map<std::pair<unsigned int, unsigned int>, unsigned int>> node_index_map(
      n_processors());

  if (!_neighbors_cached)
  {
    for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
    {
      System * to_sys = find_sys(*_to_es[i_to], _to_var_name);
      unsigned int sys_num = to_sys->number();
      unsigned int var_num = to_sys->variable_number(_to_var_name);
      MeshBase * to_mesh = &_to_meshes[i_to]->getMesh();
      bool is_to_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

      if (is_to_nodal)
      {
        std::vector<Node *> target_local_nodes;

        if (isParamValid("target_boundary"))
        {
          BoundaryID target_bnd_id =
              _to_meshes[i_to]->getBoundaryID(getParam<BoundaryName>("target_boundary"));

          ConstBndNodeRange & bnd_nodes = *(_to_meshes[i_to])->getBoundaryNodeRange();
          for (const auto & bnode : bnd_nodes)
            if (bnode->_bnd_id == target_bnd_id && bnode->_node->processor_id() == processor_id())
              target_local_nodes.push_back(bnode->_node);
        }
        else
        {
          target_local_nodes.resize(to_mesh->n_local_nodes());
          unsigned int i = 0;
          for (auto & node : to_mesh->local_node_ptr_range())
            target_local_nodes[i++] = node;
        }

        // For error checking: keep track of all target_local_nodes
        // which are successfully mapped to at least one domain where
        // the nearest neighbor might be found.
        std::set<Node *> local_nodes_found;

        for (const auto & node : target_local_nodes)
        {
          // Skip this node if the variable has no dofs at it.
          if (node->n_dofs(sys_num, var_num) < 1)
            continue;

          // Find which bboxes might have the nearest node to this point.
          Real nearest_max_distance = std::numeric_limits<Real>::max();
          for (const auto & bbox : bboxes)
          {
            Real distance = bboxMaxDistance(*node, bbox);
            if (distance < nearest_max_distance)
              nearest_max_distance = distance;
          }

          unsigned int from0 = 0;
          for (processor_id_type i_proc = 0; i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            bool qp_found = false;

            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && !qp_found;
                 i_from++)
            {

              Real distance = bboxMinDistance(*node, bboxes[i_from]);

              if (distance <= nearest_max_distance || bboxes[i_from].contains_point(*node))
              {
                std::pair<unsigned int, unsigned int> key(i_to, node->id());
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(*node + _to_positions[i_to]);
                qp_found = true;
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
                       *node);
      }
      else // Elemental
      {
        // For error checking: keep track of all local elements
        // which are successfully mapped to at least one domain where
        // the nearest neighbor might be found.
        std::set<Elem *> local_elems_found;

        for (auto & elem : as_range(to_mesh->local_elements_begin(), to_mesh->local_elements_end()))
        {
          Point centroid = elem->centroid();

          // Skip this element if the variable has no dofs at it.
          if (elem->n_dofs(sys_num, var_num) < 1)
            continue;

          // Find which bboxes might have the nearest node to this point.
          Real nearest_max_distance = std::numeric_limits<Real>::max();
          for (const auto & bbox : bboxes)
          {
            Real distance = bboxMaxDistance(centroid, bbox);
            if (distance < nearest_max_distance)
              nearest_max_distance = distance;
          }

          unsigned int from0 = 0;
          for (processor_id_type i_proc = 0; i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            bool qp_found = false;
            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && !qp_found;
                 i_from++)
            {
              Real distance = bboxMinDistance(centroid, bboxes[i_from]);
              if (distance <= nearest_max_distance || bboxes[i_from].contains_point(centroid))
              {
                std::pair<unsigned int, unsigned int> key(i_to, elem->id());
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(centroid + _to_positions[i_to]);
                qp_found = true;
                local_elems_found.insert(elem);
              }
            }
          }
        }

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
                       elem->centroid());
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

  std::vector<std::vector<Real>> incoming_evals(n_processors());
  std::vector<Parallel::Request> send_qps(n_processors());
  std::vector<Parallel::Request> send_evals(n_processors());

  // Create these here so that they live the entire life of this function
  // and are NOT reused per processor.
  std::vector<std::vector<Real>> processor_outgoing_evals(n_processors());

  if (!_neighbors_cached)
  {
    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
    {
      if (i_proc == processor_id())
        continue;
      _communicator.send(i_proc, outgoing_qps[i_proc], send_qps[i_proc]);
    }

    // Build an array of pointers to all of this processor's local entities (nodes or
    // elements).  We need to do this to avoid the expense of using LibMesh iterators.
    // This step also takes care of limiting the search to boundary nodes, if
    // applicable.
    std::vector<std::vector<std::pair<Point, DofObject *>>> local_entities(
        froms_per_proc[processor_id()]);

    // Local array of all from Variable references
    std::vector<std::reference_wrapper<MooseVariableFEBase>> _from_vars;

    for (unsigned int i = 0; i < froms_per_proc[processor_id()]; i++)
    {
      MooseVariableFEBase & from_var = _from_problems[i]->getVariable(
          0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      bool is_to_nodal = from_var.feType().family == LAGRANGE;

      _from_vars.emplace_back(from_var);
      getLocalEntities(_from_meshes[i], local_entities[i], is_to_nodal);
    }

    if (_fixed_meshes)
    {
      _cached_froms.resize(n_processors());
      _cached_dof_ids.resize(n_processors());
    }

    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
    {
      // We either use our own outgoing_qps or receive them from
      // another processor.
      std::vector<Point> incoming_qps;
      if (i_proc == processor_id())
        incoming_qps = outgoing_qps[i_proc];
      else
        _communicator.receive(i_proc, incoming_qps);

      if (_fixed_meshes)
      {
        _cached_froms[i_proc].resize(incoming_qps.size());
        _cached_dof_ids[i_proc].resize(incoming_qps.size());
      }

      std::vector<Real> & outgoing_evals = processor_outgoing_evals[i_proc];
      // Resize this vector to two times the size of the incoming_qps
      // vector because we are going to store both the value from the nearest
      // local node *and* the distance between the incoming_qp and that node
      // for later comparison purposes.
      outgoing_evals.resize(2 * incoming_qps.size());

      for (unsigned int qp = 0; qp < incoming_qps.size(); qp++)
      {
        const Point & qpt = incoming_qps[qp];
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
            // Compute distance between the current incoming_qp to local node i_node.
            Real current_distance =
                (qpt - local_entities[i_local_from][i_node].first - _from_positions[i_local_from])
                    .norm();

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
                    from_sys_num, from_var_num, 0);

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
                  _cached_froms[i_proc][qp] = i_local_from;
                  _cached_dof_ids[i_proc][qp] = from_dof;
                }
              }
            }
          }
        }
      }

      if (i_proc == processor_id())
        incoming_evals[i_proc] = outgoing_evals;
      else
        _communicator.send(i_proc, outgoing_evals, send_evals[i_proc]);
    }
  }

  else // We've cached the nearest nodes.
  {
    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
    {
      std::vector<Real> & outgoing_evals = processor_outgoing_evals[i_proc];
      outgoing_evals.resize(_cached_froms[i_proc].size());

      for (unsigned int qp = 0; qp < outgoing_evals.size(); qp++)
      {
        MooseVariableFEBase & from_var = _from_problems[_cached_froms[i_proc][qp]]->getVariable(
            0,
            _from_var_name,
            Moose::VarKindType::VAR_ANY,
            Moose::VarFieldType::VAR_FIELD_STANDARD);
        System & from_sys = from_var.sys().system();
        dof_id_type from_dof = _cached_dof_ids[i_proc][qp];
        // outgoing_evals[qp] = (*from_sys.solution)(_cached_dof_ids[i_proc][qp]);
        outgoing_evals[qp] = (*from_sys.solution)(from_dof);
      }

      if (i_proc == processor_id())
        incoming_evals[i_proc] = outgoing_evals;
      else
        _communicator.send(i_proc, outgoing_evals, send_evals[i_proc]);
    }
  }

  ////////////////////
  // Gather all of the evaluations, find the nearest one for each node/element,
  // and apply the values.
  ////////////////////

  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;

    _communicator.receive(i_proc, incoming_evals[i_proc]);
  }

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    // Loop over the master nodes and set the value of the variable
    System * to_sys = find_sys(*_to_es[i_to], _to_var_name);

    unsigned int sys_num = to_sys->number();
    unsigned int var_num = to_sys->variable_number(_to_var_name);

    NumericVector<Real> * solution = nullptr;
    switch (_direction)
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

    bool is_to_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

    if (is_to_nodal)
    {
      std::vector<Node *> target_local_nodes;

      if (isParamValid("target_boundary"))
      {
        BoundaryID target_bnd_id =
            _to_meshes[i_to]->getBoundaryID(getParam<BoundaryName>("target_boundary"));

        ConstBndNodeRange & bnd_nodes = *(_to_meshes[i_to])->getBoundaryNodeRange();
        for (const auto & bnode : bnd_nodes)
          if (bnode->_bnd_id == target_bnd_id && bnode->_node->processor_id() == processor_id())
            target_local_nodes.push_back(bnode->_node);
      }
      else
      {
        target_local_nodes.resize(to_mesh.n_local_nodes());
        unsigned int i = 0;
        for (auto & node : to_mesh.local_node_ptr_range())
          target_local_nodes[i++] = node;
      }

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
          for (unsigned int i_from = 0; i_from < incoming_evals.size(); i_from++)
          {
            std::pair<unsigned int, unsigned int> key(i_to, node->id());
            if (node_index_map[i_from].find(key) == node_index_map[i_from].end())
              continue;
            unsigned int qp_ind = node_index_map[i_from][key];
            if (incoming_evals[i_from][2 * qp_ind] >= min_dist)
              continue;

            // If we made it here, we are going set a new value and
            // distance because we found one that was closer.
            min_dist = incoming_evals[i_from][2 * qp_ind];
            best_val = incoming_evals[i_from][2 * qp_ind + 1];

            if (_fixed_meshes)
            {
              // Cache these indices.
              _cached_from_inds[node->id()] = i_from;
              _cached_qp_inds[node->id()] = qp_ind;
            }
          }
        }

        else
        {
          best_val = incoming_evals[_cached_from_inds[node->id()]][_cached_qp_inds[node->id()]];
        }

        dof_id_type dof = node->dof_number(sys_num, var_num, 0);
        solution->set(dof, best_val);
      }
    }
    else // Elemental
    {
      for (auto & elem : to_mesh.active_local_element_ptr_range())
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        Real best_val = 0;
        if (!_neighbors_cached)
        {
          Real min_dist = std::numeric_limits<Real>::max();
          for (unsigned int i_from = 0; i_from < incoming_evals.size(); i_from++)
          {
            std::pair<unsigned int, unsigned int> key(i_to, elem->id());
            if (node_index_map[i_from].find(key) == node_index_map[i_from].end())
              continue;
            unsigned int qp_ind = node_index_map[i_from][key];
            if (incoming_evals[i_from][2 * qp_ind] >= min_dist)
              continue;
            min_dist = incoming_evals[i_from][2 * qp_ind];
            best_val = incoming_evals[i_from][2 * qp_ind + 1];

            if (_fixed_meshes)
            {
              // Cache these indices.
              _cached_from_inds[elem->id()] = i_from;
              _cached_qp_inds[elem->id()] = qp_ind;
            }
          }
        }

        else
        {
          best_val = incoming_evals[_cached_from_inds[elem->id()]][_cached_qp_inds[elem->id()]];
        }

        dof_id_type dof = elem->dof_number(sys_num, var_num, 0);
        solution->set(dof, best_val);
      }
    }
    solution->close();
    to_sys->update();
  }

  if (_fixed_meshes)
    _neighbors_cached = true;

  // Make sure all our sends succeeded.
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;
    send_qps[i_proc].wait();
    send_evals[i_proc].wait();
  }

  _console << "Finished NearestNodeTransfer " << name() << std::endl;

  postExecute();
}

Node *
MultiAppNearestNodeTransfer::getNearestNode(const Point & p,
                                            Real & distance,
                                            MooseMesh * mesh,
                                            bool local)
{
  distance = std::numeric_limits<Real>::max();
  Node * nearest = nullptr;

  if (isParamValid("source_boundary"))
  {
    BoundaryID src_bnd_id = mesh->getBoundaryID(getParam<BoundaryName>("source_boundary"));

    const ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      if (bnode->_bnd_id == src_bnd_id)
      {
        Node * node = bnode->_node;
        Real current_distance = (p - *node).norm();

        if (current_distance < distance)
        {
          distance = current_distance;
          nearest = node;
        }
      }
    }
  }
  else
  {
    for (auto & node : as_range(local ? mesh->localNodesBegin() : mesh->getMesh().nodes_begin(),
                                local ? mesh->localNodesEnd() : mesh->getMesh().nodes_end()))
    {
      Real current_distance = (p - *node).norm();

      if (current_distance < distance)
      {
        distance = current_distance;
        nearest = node;
      }
    }
  }

  return nearest;
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
MultiAppNearestNodeTransfer::getLocalEntities(
    MooseMesh * mesh, std::vector<std::pair<Point, DofObject *>> & local_entities, bool is_nodal)
{
  mooseAssert(local_entities.empty(), "local_entities should be empty");
  const MeshBase & mesh_base = mesh->getMesh();

  if (isParamValid("source_boundary"))
  {
    BoundaryID src_bnd_id = mesh->getBoundaryID(getParam<BoundaryName>("source_boundary"));
    auto proc_id = processor_id();
    if (is_nodal)
    {
      const ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
      for (const auto & bnode : bnd_nodes)
        if (bnode->_bnd_id == src_bnd_id && bnode->_node->processor_id() == proc_id)
          local_entities.emplace_back(*bnode->_node, bnode->_node);
    }
    else
    {
      const ConstBndElemRange & bnd_elems = *mesh->getBoundaryElementRange();
      for (const auto & belem : bnd_elems)
        if (belem->_bnd_id == src_bnd_id && belem->_elem->processor_id() == proc_id)
          local_entities.emplace_back(belem->_elem->centroid(), belem->_elem);
    }
  }
  else
  {
    if (is_nodal)
    {
      local_entities.reserve(mesh_base.n_local_nodes());
      for (auto & node : mesh_base.local_node_ptr_range())
        local_entities.emplace_back(*node, node);
    }
    else
    {
      local_entities.reserve(mesh_base.n_local_elem());
      for (auto & elem : mesh_base.active_local_element_ptr_range())
        local_entities.emplace_back(elem->centroid(), elem);
    }
  }
}
