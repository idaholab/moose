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
#include "MultiAppProjectionTransfer.h"
#include "FEProblem.h"
#include "AddVariableAction.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dof_map.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/sparse_matrix.h"

void
assemble_l2(EquationSystems & es, const std::string & system_name)
{
  MultiAppProjectionTransfer * transfer =
      es.parameters.get<MultiAppProjectionTransfer *>("transfer");
  transfer->assembleL2(es, system_name);
}

template <>
InputParameters
validParams<MultiAppProjectionTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");

  MooseEnum proj_type("l2", "l2");
  params.addParam<MooseEnum>("proj_type", proj_type, "The type of the projection.");

  params.addParam<bool>("fixed_meshes",
                        false,
                        "Set to true when the meshes are not changing (ie, "
                        "no movement or adaptivity).  This will cache some "
                        "information to speed up the transfer.");

  return params;
}

MultiAppProjectionTransfer::MultiAppProjectionTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _proj_type(getParam<MooseEnum>("proj_type")),
    _compute_matrix(true),
    _fixed_meshes(getParam<bool>("fixed_meshes")),
    _qps_cached(false)
{
}

void
MultiAppProjectionTransfer::initialSetup()
{
  getAppInfo();

  _proj_sys.resize(_to_problems.size(), NULL);

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    FEProblemBase & to_problem = *_to_problems[i_to];
    EquationSystems & to_es = to_problem.es();

    // Add the projection system.
    FEType fe_type = to_problem.getVariable(0, _to_var_name).feType();
    LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + name());
    _proj_var_num = proj_sys.add_variable("var", fe_type);
    proj_sys.attach_assemble_function(assemble_l2);
    _proj_sys[i_to] = &proj_sys;

    // Prevent the projection system from being written to checkpoint
    // files.  In the event of a recover or restart, we'll read the checkpoint
    // before this initialSetup method is called.  As a result, we'll find
    // systems in the checkpoint (the projection systems) that we don't know
    // what to do with, and there will be a crash.  We could fix this by making
    // the systems in the constructor, except we don't know how many sub apps
    // there are at the time of construction.  So instead, we'll just nuke the
    // projection system and rebuild it from scratch every recover/restart.
    proj_sys.hide_output() = true;

    // Reinitialize EquationSystems since we added a system.
    to_es.reinit();
  }

  if (_fixed_meshes)
  {
    _cached_qps.resize(n_processors());
    _cached_index_map.resize(n_processors());
  }
}

void
MultiAppProjectionTransfer::assembleL2(EquationSystems & es, const std::string & system_name)
{
  // Get the system and mesh from the input arguments.
  LinearImplicitSystem & system = es.get_system<LinearImplicitSystem>(system_name);
  MeshBase & to_mesh = es.get_mesh();

  // Get the meshfunction evaluations and the map that was stashed in the es.
  std::vector<Real> & final_evals = *es.parameters.get<std::vector<Real> *>("final_evals");
  std::map<dof_id_type, unsigned int> & element_map =
      *es.parameters.get<std::map<dof_id_type, unsigned int> *>("element_map");

  // Setup system vectors and matrices.
  FEType fe_type = system.variable_type(0);
  std::unique_ptr<FEBase> fe(FEBase::build(to_mesh.mesh_dimension(), fe_type));
  QGauss qrule(to_mesh.mesh_dimension(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  const DofMap & dof_map = system.get_dof_map();
  DenseMatrix<Number> Ke;
  DenseVector<Number> Fe;
  std::vector<dof_id_type> dof_indices;
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<Real>> & phi = fe->get_phi();

  const MeshBase::const_element_iterator end_el = to_mesh.active_local_elements_end();
  for (MeshBase::const_element_iterator el = to_mesh.active_local_elements_begin(); el != end_el;
       ++el)
  {
    const Elem * elem = *el;
    fe->reinit(elem);

    dof_map.dof_indices(elem, dof_indices);
    Ke.resize(dof_indices.size(), dof_indices.size());
    Fe.resize(dof_indices.size());

    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
    {
      Real meshfun_eval = 0.;
      if (element_map.find(elem->id()) != element_map.end())
      {
        // We have evaluations for this element.
        meshfun_eval = final_evals[element_map[elem->id()] + qp];
      }

      // Now compute the element matrix and RHS contributions.
      for (unsigned int i = 0; i < phi.size(); i++)
      {
        // RHS
        Fe(i) += JxW[qp] * (meshfun_eval * phi[i][qp]);

        if (_compute_matrix)
          for (unsigned int j = 0; j < phi.size(); j++)
          {
            // The matrix contribution
            Ke(i, j) += JxW[qp] * (phi[i][qp] * phi[j][qp]);
          }
      }
      dof_map.constrain_element_matrix_and_vector(Ke, Fe, dof_indices);

      if (_compute_matrix)
        system.matrix->add_matrix(Ke, dof_indices);
      system.rhs->add_vector(Fe, dof_indices);
    }
  }
}

void
MultiAppProjectionTransfer::execute()
{
  _console << "Beginning projection transfer " << name() << std::endl;

  getAppInfo();

  ////////////////////
  // We are going to project the solutions by solving some linear systems.  In
  // order to assemble the systems, we need to evaluate the "from" domain
  // solutions at quadrature points in the "to" domain.  Some parallel
  // communication is necessary because each processor doesn't necessarily have
  // all the "from" information it needs to set its "to" values.  We don't want
  // to use a bunch of big all-to-all broadcasts, so we'll use bounding boxes to
  // figure out which processors have the information we need and only
  // communicate with those processors.
  //
  // Each processor will
  // 1. Check its local quadrature points in the "to" domains to see which
  //    "from" domains they might be in.
  // 2. Send quadrature points to the processors with "from" domains that might
  //    contain those points.
  // 3. Recieve quadrature points from other processors, evaluate its mesh
  //    functions at those points, and send the values back to the proper
  //    processor
  // 4. Recieve mesh function evaluations from all relevant processors and
  //    decide which one to use at every quadrature point (the lowest global app
  //    index always wins)
  // 5. And use the mesh function evaluations to assemble and solve an L2
  //    projection system on its local elements.
  ////////////////////

  ////////////////////
  // For every combination of global "from" problem and local "to" problem, find
  // which "from" bounding boxes overlap with which "to" elements.  Keep track
  // of which processors own bounding boxes that overlap with which elements.
  // Build vectors of quadrature points to send to other processors for mesh
  // function evaluations.
  ////////////////////

  // Get the bounding boxes for the "from" domains.
  std::vector<MeshTools::BoundingBox> bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  std::vector<std::vector<Point>> outgoing_qps(n_processors());
  std::vector<std::map<std::pair<unsigned int, unsigned int>, unsigned int>> element_index_map(
      n_processors());
  // element_index_map[i_to, element_id] = index
  // outgoing_qps[index] is the first quadrature point in element

  if (!_qps_cached)
  {
    for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
    {
      MeshBase & to_mesh = _to_meshes[i_to]->getMesh();

      LinearImplicitSystem & system = *_proj_sys[i_to];

      FEType fe_type = system.variable_type(0);
      std::unique_ptr<FEBase> fe(FEBase::build(to_mesh.mesh_dimension(), fe_type));
      QGauss qrule(to_mesh.mesh_dimension(), fe_type.default_quadrature_order());
      fe->attach_quadrature_rule(&qrule);
      const std::vector<Point> & xyz = fe->get_xyz();

      MeshBase::const_element_iterator el = to_mesh.local_elements_begin();
      const MeshBase::const_element_iterator end_el = to_mesh.local_elements_end();

      unsigned int from0 = 0;
      for (processor_id_type i_proc = 0; i_proc < n_processors();
           from0 += froms_per_proc[i_proc], i_proc++)
      {
        for (el = to_mesh.local_elements_begin(); el != end_el; el++)
        {
          const Elem * elem = *el;
          fe->reinit(elem);

          bool qp_hit = false;
          for (unsigned int i_from = 0; i_from < froms_per_proc[i_proc] && !qp_hit; i_from++)
          {
            for (unsigned int qp = 0; qp < qrule.n_points() && !qp_hit; qp++)
            {
              Point qpt = xyz[qp];
              if (bboxes[from0 + i_from].contains_point(qpt + _to_positions[i_to]))
                qp_hit = true;
            }
          }

          if (qp_hit)
          {
            // The selected processor's bounding box contains at least one
            // quadrature point from this element.  Send all qps from this element
            // and remember where they are in the array using the map.
            std::pair<unsigned int, unsigned int> key(i_to, elem->id());
            element_index_map[i_proc][key] = outgoing_qps[i_proc].size();
            for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
            {
              Point qpt = xyz[qp];
              outgoing_qps[i_proc].push_back(qpt + _to_positions[i_to]);
            }
          }
        }
      }
    }

    if (_fixed_meshes)
      _cached_index_map = element_index_map;
  }
  else
  {
    element_index_map = _cached_index_map;
  }

  ////////////////////
  // Request quadrature point evaluations from other processors and handle
  // requests sent to this processor.
  ////////////////////

  // Non-blocking send quadrature points to other processors.
  std::vector<Parallel::Request> send_qps(n_processors());
  if (!_qps_cached)
    for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
      if (i_proc != processor_id())
        _communicator.send(i_proc, outgoing_qps[i_proc], send_qps[i_proc]);

  // Get the local bounding boxes.
  std::vector<MeshTools::BoundingBox> local_bboxes(froms_per_proc[processor_id()]);
  {
    // Find the index to the first of this processor's local bounding boxes.
    unsigned int local_start = 0;
    for (processor_id_type i_proc = 0; i_proc < n_processors() && i_proc != processor_id();
         i_proc++)
      local_start += froms_per_proc[i_proc];

    // Extract the local bounding boxes.
    for (unsigned int i_from = 0; i_from < froms_per_proc[processor_id()]; i_from++)
      local_bboxes[i_from] = bboxes[local_start + i_from];
  }

  // Setup the local mesh functions.
  std::vector<MeshFunction *> local_meshfuns(froms_per_proc[processor_id()], NULL);
  for (unsigned int i_from = 0; i_from < _from_problems.size(); i_from++)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    MeshFunction * from_func = new MeshFunction(
        from_problem.es(), *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num);
    from_func->init(Trees::ELEMENTS);
    from_func->enable_out_of_mesh_mode(OutOfMeshValue);
    local_meshfuns[i_from] = from_func;
  }

  // Recieve quadrature points from other processors, evaluate mesh frunctions
  // at those points, and send the values back.
  std::vector<Parallel::Request> send_evals(n_processors());
  std::vector<Parallel::Request> send_ids(n_processors());
  std::vector<std::vector<Real>> outgoing_evals(n_processors());
  std::vector<std::vector<unsigned int>> outgoing_ids(n_processors());
  std::vector<std::vector<Real>> incoming_evals(n_processors());
  std::vector<std::vector<unsigned int>> incoming_app_ids(n_processors());
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    // Use the cached qps if they're available.
    std::vector<Point> incoming_qps;
    if (!_qps_cached)
    {
      if (i_proc == processor_id())
        incoming_qps = outgoing_qps[i_proc];
      else
        _communicator.receive(i_proc, incoming_qps);
      // Cache these qps for later if _fixed_meshes
      if (_fixed_meshes)
        _cached_qps[i_proc] = incoming_qps;
    }
    else
    {
      incoming_qps = _cached_qps[i_proc];
    }

    outgoing_evals[i_proc].resize(incoming_qps.size(), OutOfMeshValue);
    if (_direction == FROM_MULTIAPP)
      outgoing_ids[i_proc].resize(incoming_qps.size(), libMesh::invalid_uint);
    for (unsigned int qp = 0; qp < incoming_qps.size(); qp++)
    {
      Point qpt = incoming_qps[qp];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (unsigned int i_from = 0; i_from < _from_problems.size(); i_from++)
      {
        if (local_bboxes[i_from].contains_point(qpt))
        {
          outgoing_evals[i_proc][qp] = (*local_meshfuns[i_from])(qpt - _from_positions[i_from]);
          if (_direction == FROM_MULTIAPP)
            outgoing_ids[i_proc][qp] = _local2global_map[i_from];
        }
      }
    }

    if (i_proc == processor_id())
    {
      incoming_evals[i_proc] = outgoing_evals[i_proc];
      if (_direction == FROM_MULTIAPP)
        incoming_app_ids[i_proc] = outgoing_ids[i_proc];
    }
    else
    {
      _communicator.send(i_proc, outgoing_evals[i_proc], send_evals[i_proc]);
      if (_direction == FROM_MULTIAPP)
        _communicator.send(i_proc, outgoing_ids[i_proc], send_ids[i_proc]);
    }
  }

  ////////////////////
  // Gather all of the qp evaluations and pick out the best ones for each qp.
  ////////////////////
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;
    _communicator.receive(i_proc, incoming_evals[i_proc]);
    if (_direction == FROM_MULTIAPP)
      _communicator.receive(i_proc, incoming_app_ids[i_proc]);
  }

  std::vector<std::vector<Real>> final_evals(_to_problems.size());
  std::vector<std::map<dof_id_type, unsigned int>> trimmed_element_maps(_to_problems.size());

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    MeshBase & to_mesh = _to_meshes[i_to]->getMesh();
    LinearImplicitSystem & system = *_proj_sys[i_to];

    FEType fe_type = system.variable_type(0);
    std::unique_ptr<FEBase> fe(FEBase::build(to_mesh.mesh_dimension(), fe_type));
    QGauss qrule(to_mesh.mesh_dimension(), fe_type.default_quadrature_order());
    fe->attach_quadrature_rule(&qrule);
    const std::vector<Point> & xyz = fe->get_xyz();

    MeshBase::const_element_iterator el = to_mesh.local_elements_begin();
    const MeshBase::const_element_iterator end_el = to_mesh.local_elements_end();

    for (el = to_mesh.active_local_elements_begin(); el != end_el; el++)
    {
      const Elem * elem = *el;
      fe->reinit(elem);

      bool element_is_evaled = false;
      std::vector<Real> evals(qrule.n_points(), 0.);

      for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      {
        Point qpt = xyz[qp];

        unsigned int lowest_app_rank = libMesh::invalid_uint;
        for (unsigned int i_proc = 0; i_proc < n_processors(); i_proc++)
        {
          // Ignore the selected processor if the element wasn't found in it's
          // bounding box.
          std::map<std::pair<unsigned int, unsigned int>, unsigned int> & map =
              element_index_map[i_proc];
          std::pair<unsigned int, unsigned int> key(i_to, elem->id());
          if (map.find(key) == map.end())
            continue;
          unsigned int qp0 = map[key];

          // Ignore the selected processor if it's app has a higher rank than the
          // previously found lowest app rank.
          if (_direction == FROM_MULTIAPP)
            if (incoming_app_ids[i_proc][qp0 + qp] >= lowest_app_rank)
              continue;

          // Ignore the selected processor if the qp was actually outside the
          // processor's subapp's mesh.
          if (incoming_evals[i_proc][qp0 + qp] == OutOfMeshValue)
            continue;

          // This is the best meshfunction evaluation so far, save it.
          element_is_evaled = true;
          evals[qp] = incoming_evals[i_proc][qp0 + qp];
        }
      }

      // If we found good evaluations for any of the qps in this element, save
      // those evaluations for later.
      if (element_is_evaled)
      {
        trimmed_element_maps[i_to][elem->id()] = final_evals[i_to].size();
        for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
          final_evals[i_to].push_back(evals[qp]);
      }
    }
  }

  ////////////////////
  // We now have just one or zero mesh function values at all of our local
  // quadrature points.  Stash those values (and a map linking them to element
  // ids) in the equation systems parameters and project the solution.
  ////////////////////

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    _to_es[i_to]->parameters.set<std::vector<Real> *>("final_evals") = &final_evals[i_to];
    _to_es[i_to]->parameters.set<std::map<dof_id_type, unsigned int> *>("element_map") =
        &trimmed_element_maps[i_to];
    projectSolution(i_to);
    _to_es[i_to]->parameters.set<std::vector<Real> *>("final_evals") = NULL;
    _to_es[i_to]->parameters.set<std::map<dof_id_type, unsigned int> *>("element_map") = NULL;
  }

  for (unsigned int i = 0; i < _from_problems.size(); i++)
    delete local_meshfuns[i];

  // Make sure all our sends succeeded.
  for (processor_id_type i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;
    if (!_qps_cached)
      send_qps[i_proc].wait();
    send_evals[i_proc].wait();
    if (_direction == FROM_MULTIAPP)
      send_ids[i_proc].wait();
  }

  if (_fixed_meshes)
    _qps_cached = true;

  _console << "Finished projection transfer " << name() << std::endl;
}

void
MultiAppProjectionTransfer::projectSolution(unsigned int i_to)
{
  FEProblemBase & to_problem = *_to_problems[i_to];
  EquationSystems & proj_es = to_problem.es();
  LinearImplicitSystem & ls = *_proj_sys[i_to];
  // activate the current transfer
  proj_es.parameters.set<MultiAppProjectionTransfer *>("transfer") = this;

  // TODO: specify solver params in an input file
  // solver tolerance
  Real tol = proj_es.parameters.get<Real>("linear solver tolerance");
  proj_es.parameters.set<Real>("linear solver tolerance") = 1e-10; // set our tolerance
  // solve it
  ls.solve();
  proj_es.parameters.set<Real>("linear solver tolerance") = tol; // restore the original tolerance

  // copy projected solution into target es
  MeshBase & to_mesh = proj_es.get_mesh();

  MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
  System & to_sys = to_var.sys().system();
  NumericVector<Number> * to_solution = to_sys.solution.get();

  {
    MeshBase::const_node_iterator it = to_mesh.local_nodes_begin();
    const MeshBase::const_node_iterator end_it = to_mesh.local_nodes_end();
    for (; it != end_it; ++it)
    {
      const Node * node = *it;
      for (unsigned int comp = 0; comp < node->n_comp(to_sys.number(), to_var.number()); comp++)
      {
        const dof_id_type proj_index = node->dof_number(ls.number(), _proj_var_num, comp);
        const dof_id_type to_index = node->dof_number(to_sys.number(), to_var.number(), comp);
        to_solution->set(to_index, (*ls.solution)(proj_index));
      }
    }
  }
  {
    MeshBase::const_element_iterator it = to_mesh.active_local_elements_begin();
    const MeshBase::const_element_iterator end_it = to_mesh.active_local_elements_end();
    for (; it != end_it; ++it)
    {
      const Elem * elem = *it;
      for (unsigned int comp = 0; comp < elem->n_comp(to_sys.number(), to_var.number()); comp++)
      {
        const dof_id_type proj_index = elem->dof_number(ls.number(), _proj_var_num, comp);
        const dof_id_type to_index = elem->dof_number(to_sys.number(), to_var.number(), comp);
        to_solution->set(to_index, (*ls.solution)(proj_index));
      }
    }
  }

  to_solution->close();
  to_sys.update();
}
