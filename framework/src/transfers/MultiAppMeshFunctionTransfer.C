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
#include "MultiAppMeshFunctionTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MooseTypes.h" // for MooseSharedPointer
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_algebra.h" // for communicator send and recieve stuff

template<>
InputParameters validParams<MultiAppMeshFunctionTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("displaced_source_mesh", false, "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh", false, "Whether or not to use the displaced mesh for the target mesh.");
  params.addParam<bool>("error_on_miss", false, "Whether or not to error in the case that a target point is not found in the source domain.");
  return params;
}

MultiAppMeshFunctionTransfer::MultiAppMeshFunctionTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _error_on_miss(getParam<bool>("error_on_miss"))
{
  _displaced_source_mesh = getParam<bool>("displaced_source_mesh");
  _displaced_target_mesh = getParam<bool>("displaced_target_mesh");
}

void
MultiAppMeshFunctionTransfer::initialSetup()
{
  if (_direction == TO_MULTIAPP)
    variableIntegrityCheck(_to_var_name);
  else
    variableIntegrityCheck(_from_var_name);
}

void
MultiAppMeshFunctionTransfer::execute()
{
  Moose::out << "Beginning MeshFunctionTransfer " << name() << std::endl;

  getAppInfo();

  /**
   * For every combination of global "from" problem and local "to" problem, find
   * which "from" bounding boxes overlap with which "to" elements.  Keep track
   * of which processors own bounding boxes that overlap with which elements.
   * Build vectors of node locations/element centroids to send to other
   * processors for mesh function evaluations.
   */

  // Get the bounding boxes for the "from" domains.
  std::vector<MeshTools::BoundingBox> bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  std::vector<std::vector<Point> > outgoing_points(n_processors());
  std::vector<std::map<std::pair<unsigned int, unsigned int>, unsigned int> > point_index_map(n_processors());
  // point_index_map[i_to, element_id] = index
  // outgoing_points[index] is the first quadrature point in element

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    System * to_sys = find_sys(*_to_es[i_to], _to_var_name);
    unsigned int sys_num = to_sys->number();
    unsigned int var_num = to_sys->variable_number(_to_var_name);
    MeshBase * to_mesh = & _to_meshes[i_to]->getMesh();
    bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

    if (is_nodal)
    {
      MeshBase::const_node_iterator node_it = to_mesh->local_nodes_begin();
      MeshBase::const_node_iterator node_end = to_mesh->local_nodes_end();

      for (; node_it != node_end; ++node_it)
      {
        Node * node = *node_it;

        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        // Loop over the "froms" on processor i_proc.  If the node is found in
        // any of the "froms", add that node to the vector that will be sent to
        // i_proc.
        unsigned int from0 = 0;
        for (processor_id_type i_proc = 0;
             i_proc < n_processors();
             from0 += froms_per_proc[i_proc], i_proc++)
        {
          bool point_found = false;
          for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && ! point_found; i_from++)
          {
            if (bboxes[i_from].contains_point(*node + _to_positions[i_to]))
            {
              std::pair<unsigned int, unsigned int> key(i_to, node->id());
              point_index_map[i_proc][key] = outgoing_points[i_proc].size();
              outgoing_points[i_proc].push_back(*node + _to_positions[i_to]);
              point_found = true;
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

        // Loop over the "froms" on processor i_proc.  If the elem is found in
        // any of the "froms", add that elem to the vector that will be sent to
        // i_proc.
        unsigned int from0 = 0;
        for (processor_id_type i_proc = 0;
             i_proc < n_processors();
             from0 += froms_per_proc[i_proc], i_proc++)
        {
          bool point_found = false;
          for (unsigned int i_from = from0; i_from < from0 + froms_per_proc[i_proc] && ! point_found; i_from++)
          {
            if (bboxes[i_from].contains_point(centroid + _to_positions[i_to]))
            {
              std::pair<unsigned int, unsigned int> key(i_to, elem->id());
              point_index_map[i_proc][key] = outgoing_points[i_proc].size();
              outgoing_points[i_proc].push_back(centroid + _to_positions[i_to]);
              point_found = true;
            }
          }
        }
      }
    }
  }

  /**
   * Request point evaluations from other processors and handle requests sent to
   * this processor.
   */

  // Get the local bounding boxes.
  std::vector<MeshTools::BoundingBox> local_bboxes(froms_per_proc[processor_id()]);
  {
    // Find the index to the first of this processor's local bounding boxes.
    unsigned int local_start = 0;
    for (processor_id_type i_proc = 0;
         i_proc < n_processors() && i_proc != processor_id();
         i_proc++)
    {
      local_start += froms_per_proc[i_proc];
    }

    // Extract the local bounding boxes.
    for (unsigned int i_from = 0; i_from < froms_per_proc[processor_id()]; i_from++)
    {
      local_bboxes[i_from] = bboxes[local_start + i_from];
    }
  }

  // Setup the local mesh functions.
  std::vector<MooseSharedPointer<MeshFunction> > local_meshfuns;
  for (unsigned int i_from = 0; i_from < _from_problems.size(); i_from++)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    MooseSharedPointer<MeshFunction> from_func;
    //TODO: make MultiAppTransfer give me the right es
    if (_displaced_source_mesh && from_problem.getDisplacedProblem())
      from_func.reset(new MeshFunction(from_problem.getDisplacedProblem()->es(),
           *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num));
    else
      from_func.reset(new MeshFunction(from_problem.es(),
           *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num));
    from_func->init(Trees::ELEMENTS);
    from_func->enable_out_of_mesh_mode(OutOfMeshValue);
    local_meshfuns.push_back(from_func);
  }

  // Send points to other processors.
  std::vector<std::vector<Real> > incoming_evals(n_processors());
  std::vector<std::vector<unsigned int> > incoming_app_ids(n_processors());
  std::vector<Parallel::Request> send_points(n_processors());
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;
    _communicator.send(i_proc, outgoing_points[i_proc], send_points[i_proc]);
  }

  // Recieve points from other processors, evaluate mesh frunctions at those
  // points, and send the values back.
  std::vector<Parallel::Request> send_evals(n_processors());
  std::vector<Parallel::Request> send_ids(n_processors());
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    std::vector<Point> incoming_points;
    if (i_proc == processor_id())
      incoming_points = outgoing_points[i_proc];
    else
      _communicator.receive(i_proc, incoming_points);

    std::vector<Real> outgoing_evals(incoming_points.size(), OutOfMeshValue);
    std::vector<unsigned int> outgoing_ids(incoming_points.size(), -1); // -1 = largest unsigned int
    for (unsigned int i_pt = 0; i_pt < incoming_points.size(); i_pt++)
    {
      Point pt = incoming_points[i_pt];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (unsigned int i_from = 0; i_from < _from_problems.size() && outgoing_evals[i_pt] == OutOfMeshValue; i_from++)
      {
        if (local_bboxes[i_from].contains_point(pt))
        {
          outgoing_evals[i_pt] = (* local_meshfuns[i_from])(pt - _from_positions[i_from]);
          if (_direction == FROM_MULTIAPP)
            outgoing_ids[i_pt] = _local2global_map[i_from];
        }
      }
    }

    if (i_proc == processor_id())
    {
      incoming_evals[i_proc] = outgoing_evals;
      if (_direction == FROM_MULTIAPP)
        incoming_app_ids[i_proc] = outgoing_ids;
    }
    else
    {
      _communicator.send(i_proc, outgoing_evals, send_evals[i_proc]);
      if (_direction == FROM_MULTIAPP)
        _communicator.send(i_proc, outgoing_ids, send_ids[i_proc]);
    }
  }

  /**
   * Gather all of the evaluations, pick out the best ones for each point, and
   * apply them to the solution vector.  When we are transferring from
   * multiapps, there may be multiple overlapping apps for a particular point.
   * In that case, we'll try to use the value from the app with the lowest id.
   */

  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;

    _communicator.receive(i_proc, incoming_evals[i_proc]);
    if (_direction == FROM_MULTIAPP)
      _communicator.receive(i_proc, incoming_app_ids[i_proc]);
  }

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
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
      MeshBase::const_node_iterator node_it = to_mesh->local_nodes_begin();
      MeshBase::const_node_iterator node_end = to_mesh->local_nodes_end();

      for (; node_it != node_end; ++node_it)
      {
        Node * node = *node_it;

        // Skip this node if the variable has no dofs at it.
        if (node->n_dofs(sys_num, var_num) < 1)
          continue;

        unsigned int lowest_app_rank = libMesh::invalid_uint;
        Real best_val = 0.;
        bool point_found = false;
        for (unsigned int i_proc = 0; i_proc < incoming_evals.size(); i_proc++)
        {
          // Skip this proc if the node wasn't in it's bounding boxes.
          std::pair<unsigned int, unsigned int> key(i_to, node->id());
          if (point_index_map[i_proc].find(key) == point_index_map[i_proc].end())
            continue;
          unsigned int i_pt = point_index_map[i_proc][key];

          // Ignore this proc if it's app has a higher rank than the
          // previously found lowest app rank.
          if (_direction == FROM_MULTIAPP)
          {
            if (incoming_app_ids[i_proc][i_pt] >= lowest_app_rank)
              continue;
          }

          // Ignore this proc if the point was actually outside its meshes.
          if (incoming_evals[i_proc][i_pt] == OutOfMeshValue)
            continue;

          best_val = incoming_evals[i_proc][i_pt];
          point_found = true;
        }

        if (_error_on_miss && ! point_found)
          mooseError("Point not found! " << *node + _to_positions[i_to]);

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

        unsigned int lowest_app_rank = libMesh::invalid_uint;
        Real best_val = 0;
        bool point_found = false;
        for (unsigned int i_proc = 0; i_proc < incoming_evals.size(); i_proc++)
        {
          // Skip this proc if the elem wasn't in it's bounding boxes.
          std::pair<unsigned int, unsigned int> key(i_to, elem->id());
          if (point_index_map[i_proc].find(key) == point_index_map[i_proc].end())
            continue;
          unsigned int i_pt = point_index_map[i_proc][key];

          // Ignore this proc if it's app has a higher rank than the
          // previously found lowest app rank.
          if (_direction == FROM_MULTIAPP)
          {
            if (incoming_app_ids[i_proc][i_pt] >= lowest_app_rank)
              continue;
          }

          // Ignore this proc if the point was actually outside its meshes.
          if (incoming_evals[i_proc][i_pt] == OutOfMeshValue)
            continue;

          best_val = incoming_evals[i_proc][i_pt];
          point_found = true;
        }

        if (_error_on_miss && ! point_found)
          mooseError("Point not found! " << elem->centroid() + _to_positions[i_to]);

        dof_id_type dof = elem->dof_number(sys_num, var_num, 0);
        solution->set(dof, best_val);
      }
    }
    solution->close();
    to_sys->update();
  }

  // Make sure all our sends succeeded.
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;
    send_points[i_proc].wait();
    send_evals[i_proc].wait();
    if (_direction == FROM_MULTIAPP)
      send_ids[i_proc].wait();
  }

  _console << "Finished MeshFunctionTransfer " << name() << std::endl;
}
