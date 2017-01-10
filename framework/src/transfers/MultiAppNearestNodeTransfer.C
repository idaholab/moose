/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MultiAppNearestNodeTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"
#include "libmesh/parallel_algebra.h"

template<>
InputParameters validParams<MultiAppNearestNodeTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<BoundaryName>("source_boundary", "The boundary we are transferring from (if not specified, whole domain is used).");
  params.addParam<BoundaryName>("target_boundary", "The boundary we are transferring to (if not specified, whole domain is used).");
  params.addParam<bool>("displaced_source_mesh", false, "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh", false, "Whether or not to use the displaced mesh for the target mesh.");
  params.addParam<bool>("fixed_meshes", false, "Set to true when the meshes are not changing (ie, no movement or adaptivity).  This will cache nearest node neighbors to greatly speed up the transfer.");

  return params;
}

MultiAppNearestNodeTransfer::MultiAppNearestNodeTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _fixed_meshes(getParam<bool>("fixed_meshes")),
    _node_map(declareRestartableData<std::map<dof_id_type, Node *> >("node_map")),
    _distance_map(declareRestartableData<std::map<dof_id_type, Real> >("distance_map")),
    _neighbors_cached(declareRestartableData<bool>("neighbors_cached", false)),
    _cached_froms(declareRestartableData<std::vector< std::vector<unsigned int> > >("cached_froms")),
    _cached_dof_ids(declareRestartableData<std::vector< std::vector<dof_id_type> > >("cached_dof_ids")),
    _cached_from_inds(declareRestartableData<std::map<dof_id_type, unsigned int> >("cached_from_ids")),
    _cached_qp_inds(declareRestartableData<std::map<dof_id_type, unsigned int> >("cached_qp_inds"))
{
  // This transfer does not work with DistributedMesh
  _displaced_source_mesh = getParam<bool>("displaced_source_mesh");
  _displaced_target_mesh = getParam<bool>("displaced_target_mesh");
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
  std::vector<MeshTools::BoundingBox> bboxes = getFromBoundingBoxes();

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
  std::vector<std::vector<Point> > outgoing_qps(n_processors());
  // When we get results back, node_index_map will tell us which results go with
  // which points
  std::vector<std::map<std::pair<unsigned int, unsigned int>, unsigned int> > node_index_map(n_processors());

  if (! _neighbors_cached)
  {
    for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
    {
      System * to_sys = find_sys(*_to_es[i_to], _to_var_name);
      unsigned int sys_num = to_sys->number();
      unsigned int var_num = to_sys->variable_number(_to_var_name);
      MeshBase * to_mesh = & _to_meshes[i_to]->getMesh();
      bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

      if (is_nodal)
      {
        std::vector<Node *> target_local_nodes;

        if (isParamValid("target_boundary"))
        {
          BoundaryID target_bnd_id = _to_meshes[i_to]->getBoundaryID(getParam<BoundaryName>("target_boundary"));

          ConstBndNodeRange & bnd_nodes = *(_to_meshes[i_to])->getBoundaryNodeRange();
          for (const auto & bnode : bnd_nodes)
            if (bnode->_bnd_id == target_bnd_id && bnode->_node->processor_id() == processor_id())
              target_local_nodes.push_back(bnode->_node);
        }
        else
        {
          target_local_nodes.resize(to_mesh->n_local_nodes());
          MeshBase::const_node_iterator nodes_begin = to_mesh->local_nodes_begin();
          MeshBase::const_node_iterator nodes_end = to_mesh->local_nodes_end();

          unsigned int i = 0;
          for (MeshBase::const_node_iterator nodes_it = nodes_begin; nodes_it != nodes_end; ++nodes_it, ++i)
            target_local_nodes[i] = *nodes_it;
        }

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
          for (processor_id_type i_proc = 0;
               i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            bool qp_found = false;
            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && ! qp_found; i_from++)
            {
              Real distance = bboxMinDistance(*node, bboxes[i_from]);
              if (distance < nearest_max_distance || bboxes[i_from].contains_point(*node))
              {
                std::pair<unsigned int, unsigned int> key(i_to, node->id());
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(*node + _to_positions[i_to]);
                qp_found = true;
              }
            }
          }
        }
      }
      else // Elemental
      {
        MeshBase::const_element_iterator elem_it = to_mesh->local_elements_begin();
        MeshBase::const_element_iterator elem_end = to_mesh->local_elements_end();

        for (; elem_it != elem_end; ++elem_it)
        {
          Elem * elem = *elem_it;

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
          for (processor_id_type i_proc = 0;
               i_proc < n_processors();
               from0 += froms_per_proc[i_proc], i_proc++)
          {
            bool qp_found = false;
            for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && ! qp_found; i_from++)
            {
              Real distance = bboxMinDistance(centroid, bboxes[i_from]);
              if (distance < nearest_max_distance || bboxes[i_from].contains_point(centroid))
              {
                std::pair<unsigned int, unsigned int> key(i_to, elem->id());
                node_index_map[i_proc][key] = outgoing_qps[i_proc].size();
                outgoing_qps[i_proc].push_back(centroid + _to_positions[i_to]);
                qp_found = true;
              }
            }
          }
        }
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

  std::vector<std::vector<Real> > incoming_evals(n_processors());
  std::vector<Parallel::Request> send_qps(n_processors());
  std::vector<Parallel::Request> send_evals(n_processors());
  if (! _neighbors_cached)
  {
    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
    {
      if (i_proc == processor_id())
        continue;
      _communicator.send(i_proc, outgoing_qps[i_proc], send_qps[i_proc]);
    }

    // Build an array of pointers to all of this processor's local nodes.  We
    // need to do this to avoid the expense of using LibMesh iterators.  This
    // step also takes care of limiting the search to boundary nodes, if
    // applicable.
    std::vector< std::vector<Node *> > local_nodes(froms_per_proc[processor_id()]);
    for (unsigned int i = 0; i < froms_per_proc[processor_id()]; i++)
    {
      getLocalNodes(_from_meshes[i], local_nodes[i]);
    }

    if (_fixed_meshes)
    {
      _cached_froms.resize(n_processors());
      _cached_dof_ids.resize(n_processors());
    }

    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
    {
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

      std::vector<Real> outgoing_evals(2 * incoming_qps.size());
      for (unsigned int qp = 0; qp < incoming_qps.size(); qp++)
      {
        Point qpt = incoming_qps[qp];
        outgoing_evals[2*qp] = std::numeric_limits<Real>::max();
        for (unsigned int i_local_from = 0; i_local_from < froms_per_proc[processor_id()]; i_local_from++)
        {
          MooseVariable & from_var = _from_problems[i_local_from]->getVariable(0, _from_var_name);
          System & from_sys = from_var.sys().system();
          unsigned int from_sys_num = from_sys.number();
          unsigned int from_var_num = from_sys.variable_number(from_var.name());

          for (unsigned int i_node = 0; i_node < local_nodes[i_local_from].size(); i_node++)
          {
            Real current_distance = (qpt - *(local_nodes[i_local_from][i_node]) - _from_positions[i_local_from]).norm();
            if (current_distance < outgoing_evals[2*qp])
            {
              // Assuming LAGRANGE!
              if (local_nodes[i_local_from][i_node]->n_dofs(from_sys_num, from_var_num) > 0)
              {
                dof_id_type from_dof = local_nodes[i_local_from][i_node]->dof_number(from_sys_num, from_var_num, 0);

                outgoing_evals[2*qp] = current_distance;
                outgoing_evals[2*qp + 1] = (*from_sys.solution)(from_dof);

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
      std::vector<Real> outgoing_evals(_cached_froms[i_proc].size());
      for (unsigned int qp = 0; qp < outgoing_evals.size(); qp++)
      {
        MooseVariable & from_var = _from_problems[_cached_froms[i_proc][qp]]->getVariable(0, _from_var_name);
        System & from_sys = from_var.sys().system();
        dof_id_type from_dof = _cached_dof_ids[i_proc][qp];
        //outgoing_evals[qp] = (*from_sys.solution)(_cached_dof_ids[i_proc][qp]);
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
        solution = & getTransferVector(i_to, _to_var_name);
        break;
      case FROM_MULTIAPP:
        solution = to_sys->solution.get();
        break;
    }

    MeshBase * to_mesh = & _to_meshes[i_to]->getMesh();

    bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

    if (is_nodal)
    {
      std::vector<Node *> target_local_nodes;

      if (isParamValid("target_boundary"))
      {
        BoundaryID target_bnd_id = _to_meshes[i_to]->getBoundaryID(getParam<BoundaryName>("target_boundary"));

        ConstBndNodeRange & bnd_nodes = *(_to_meshes[i_to])->getBoundaryNodeRange();
        for (const auto & bnode : bnd_nodes)
          if (bnode->_bnd_id == target_bnd_id && bnode->_node->processor_id() == processor_id())
            target_local_nodes.push_back(bnode->_node);
      }
      else
      {
        target_local_nodes.resize(to_mesh->n_local_nodes());
        MeshBase::const_node_iterator nodes_begin = to_mesh->local_nodes_begin();
        MeshBase::const_node_iterator nodes_end = to_mesh->local_nodes_end();

        unsigned int i = 0;
        for (MeshBase::const_node_iterator nodes_it = nodes_begin; nodes_it != nodes_end; ++nodes_it, ++i)
          target_local_nodes[i] = *nodes_it;
      }

      for (const auto & node : target_local_nodes)
      {
        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        Real best_val = 0;
        if (! _neighbors_cached)
        {
          Real min_dist = std::numeric_limits<Real>::max();
          for (unsigned int i_from = 0; i_from < incoming_evals.size(); i_from++)
          {
            std::pair<unsigned int, unsigned int> key(i_to, node->id());
            if (node_index_map[i_from].find(key) == node_index_map[i_from].end())
              continue;
            unsigned int qp_ind = node_index_map[i_from][key];
            if (incoming_evals[i_from][2*qp_ind] >= min_dist)
              continue;
            min_dist = incoming_evals[i_from][2*qp_ind];
            best_val = incoming_evals[i_from][2*qp_ind + 1];

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
      MeshBase::const_element_iterator elem_it = to_mesh->local_elements_begin();
      MeshBase::const_element_iterator elem_end = to_mesh->local_elements_end();

      for (; elem_it != elem_end; ++elem_it)
      {
        Elem * elem = *elem_it;

        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;

        Real best_val = 0;
        if (! _neighbors_cached)
        {
          Real min_dist = std::numeric_limits<Real>::max();
          for (unsigned int i_from = 0; i_from < incoming_evals.size(); i_from++)
          {
            std::pair<unsigned int, unsigned int> key(i_to, elem->id());
            if (node_index_map[i_from].find(key) == node_index_map[i_from].end())
              continue;
            unsigned int qp_ind = node_index_map[i_from][key];
            if (incoming_evals[i_from][2*qp_ind] >= min_dist)
              continue;
            min_dist = incoming_evals[i_from][2*qp_ind];
            best_val = incoming_evals[i_from][2*qp_ind + 1];

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
}

Node *
MultiAppNearestNodeTransfer::getNearestNode(const Point & p, Real & distance, MooseMesh * mesh, bool local)
{
  distance = std::numeric_limits<Real>::max();
  Node * nearest = NULL;

  if (isParamValid("source_boundary"))
  {
    BoundaryID src_bnd_id = mesh->getBoundaryID(getParam<BoundaryName>("source_boundary"));

    ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
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
    MeshBase::const_node_iterator nodes_begin = local ? mesh->localNodesBegin() : mesh->getMesh().nodes_begin();
    MeshBase::const_node_iterator nodes_end   = local ? mesh->localNodesEnd()   : mesh->getMesh().nodes_end();

    for (MeshBase::const_node_iterator node_it = nodes_begin; node_it != nodes_end; ++node_it)
    {
      Real current_distance = (p - *(*node_it)).norm();

      if (current_distance < distance)
      {
        distance = current_distance;
        nearest = *node_it;
      }
    }
  }

  return nearest;
}

Real
MultiAppNearestNodeTransfer::bboxMaxDistance(Point p, MeshTools::BoundingBox bbox)
{
  std::vector<Point> source_points = {bbox.first, bbox.second};

  std::vector<Point> all_points(8);
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2*y + 4*z] = Point(source_points[x](0), source_points[y](1), source_points[z](2));

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
MultiAppNearestNodeTransfer::bboxMinDistance(Point p, MeshTools::BoundingBox bbox)
{
  std::vector<Point> source_points = {bbox.first, bbox.second};

  std::vector<Point> all_points(8);
  for (unsigned int x = 0; x < 2; x++)
    for (unsigned int y = 0; y < 2; y++)
      for (unsigned int z = 0; z < 2; z++)
        all_points[x + 2*y + 4*z] = Point(source_points[x](0), source_points[y](1), source_points[z](2));

  Real min_distance = 0.;

  for (unsigned int i = 0; i < 8; i++)
  {
    Real distance = (p - all_points[i]).norm();
    if (distance < min_distance)
      min_distance = distance;
  }
  return min_distance;
}

void
MultiAppNearestNodeTransfer::getLocalNodes(MooseMesh * mesh, std::vector<Node *> & local_nodes)
{
  if (isParamValid("source_boundary"))
  {
    BoundaryID src_bnd_id = mesh->getBoundaryID(getParam<BoundaryName>("source_boundary"));

    ConstBndNodeRange & bnd_nodes = *mesh->getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
      if (bnode->_bnd_id == src_bnd_id && bnode->_node->processor_id() == processor_id())
        local_nodes.push_back(bnode->_node);
  }
  else
  {
    local_nodes.resize(mesh->getMesh().n_local_nodes());

    MeshBase::const_node_iterator nodes_begin = mesh->localNodesBegin();
    MeshBase::const_node_iterator nodes_end   = mesh->localNodesEnd();

    unsigned int i = 0;
    for (MeshBase::const_node_iterator node_it = nodes_begin; node_it != nodes_end; ++node_it, ++i)
      local_nodes[i] = *node_it;
  }
}
