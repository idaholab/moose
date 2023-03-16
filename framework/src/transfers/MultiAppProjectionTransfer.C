//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppProjectionTransfer.h"

// MOOSE includes
#include "AddVariableAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/dof_map.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/default_coupling.h"

// TIMPI includes
#include "timpi/parallel_sync.h"

void
assemble_l2(EquationSystems & es, const std::string & system_name)
{
  MultiAppProjectionTransfer * transfer =
      es.parameters.get<MultiAppProjectionTransfer *>("transfer");
  transfer->assembleL2(es, system_name);
}

registerMooseObject("MooseApp", MultiAppProjectionTransfer);

InputParameters
MultiAppProjectionTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Perform a projection between a master and sub-application mesh of a field variable.");

  MooseEnum proj_type("l2", "l2");
  params.addParam<MooseEnum>("proj_type", proj_type, "The type of the projection.");

  params.addParam<bool>("fixed_meshes",
                        false,
                        "Set to true when the meshes are not changing (ie, "
                        "no movement or adaptivity).  This will cache some "
                        "information to speed up the transfer.");

  // Need one layer of ghosting
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  MultiAppTransfer::addBBoxFactorParam(params);
  return params;
}

MultiAppProjectionTransfer::MultiAppProjectionTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _proj_type(getParam<MooseEnum>("proj_type")),
    _compute_matrix(true),
    _fixed_meshes(getParam<bool>("fixed_meshes")),
    _qps_cached(false)
{
  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() != 1)
    paramError("source_variable", " Support single from-variable only ");
}

void
MultiAppProjectionTransfer::initialSetup()
{
  MultiAppConservativeTransfer::initialSetup();

  _proj_sys.resize(_to_problems.size(), NULL);

  for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
  {
    FEProblemBase & to_problem = *_to_problems[i_to];
    EquationSystems & to_es = to_problem.es();

    // Add the projection system.
    FEType fe_type = to_problem
                         .getVariable(0,
                                      _to_var_name,
                                      Moose::VarKindType::VAR_ANY,
                                      Moose::VarFieldType::VAR_FIELD_STANDARD)
                         .feType();

    LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + name());

    proj_sys.get_dof_map().add_coupling_functor(
        proj_sys.get_dof_map().default_coupling(),
        false); // The false keeps it from getting added to the mesh

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
  auto & system_matrix = system.get_system_matrix();

  for (const auto & elem : to_mesh.active_local_element_ptr_range())
  {
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
        system_matrix.add_matrix(Ke, dof_indices);
      system.rhs->add_vector(Fe, dof_indices);
    }
  }
}

void
MultiAppProjectionTransfer::execute()
{
  TIME_SECTION(
      "MultiAppProjectionTransfer::execute()", 5, "Transferring variables through projection");

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
  std::vector<BoundingBox> bboxes = getFromBoundingBoxes();

  // Figure out how many "from" domains each processor owns.
  std::vector<unsigned int> froms_per_proc = getFromsPerProc();

  std::map<processor_id_type, std::vector<Point>> outgoing_qps;
  std::map<processor_id_type, std::map<std::pair<unsigned int, unsigned int>, unsigned int>>
      element_index_map;
  // element_index_map[i_to, element_id] = index
  // outgoing_qps[index] is the first quadrature point in element

  if (!_qps_cached)
  {
    for (unsigned int i_to = 0; i_to < _to_problems.size(); i_to++)
    {
      // Indexing into the coordinate transforms vector
      const auto to_global_num =
          _current_direction == FROM_MULTIAPP ? 0 : _to_local2global_map[i_to];
      MeshBase & to_mesh = _to_meshes[i_to]->getMesh();

      LinearImplicitSystem & system = *_proj_sys[i_to];

      FEType fe_type = system.variable_type(0);
      std::unique_ptr<FEBase> fe(FEBase::build(to_mesh.mesh_dimension(), fe_type));
      QGauss qrule(to_mesh.mesh_dimension(), fe_type.default_quadrature_order());
      fe->attach_quadrature_rule(&qrule);
      const std::vector<Point> & xyz = fe->get_xyz();

      unsigned int from0 = 0;
      for (processor_id_type i_proc = 0; i_proc < n_processors();
           from0 += froms_per_proc[i_proc], i_proc++)
      {
        for (const auto & elem :
             as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
        {
          fe->reinit(elem);

          bool qp_hit = false;
          for (unsigned int i_from = 0; i_from < froms_per_proc[i_proc] && !qp_hit; i_from++)
          {
            for (unsigned int qp = 0; qp < qrule.n_points() && !qp_hit; qp++)
            {
              Point qpt = xyz[qp];
              if (bboxes[from0 + i_from].contains_point((*_to_transforms[to_global_num])(qpt)))
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
              outgoing_qps[i_proc].push_back((*_to_transforms[to_global_num])(qpt));
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

  // Get the local bounding boxes.
  std::vector<BoundingBox> local_bboxes(froms_per_proc[processor_id()]);
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
  std::vector<MeshFunction> local_meshfuns;
  for (unsigned int i_from = 0; i_from < _from_problems.size(); i_from++)
  {
    FEProblemBase & from_problem = *_from_problems[i_from];
    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    local_meshfuns.emplace_back(
        from_problem.es(), *from_sys.current_local_solution, from_sys.get_dof_map(), from_var_num);
    local_meshfuns.back().init();
    local_meshfuns.back().enable_out_of_mesh_mode(OutOfMeshValue);
  }

  // Recieve quadrature points from other processors, evaluate mesh frunctions
  // at those points, and send the values back.
  std::map<processor_id_type, std::vector<std::pair<Real, unsigned int>>> outgoing_evals_ids;

  // If there is no cached data, we need to do communication
  // Quadrature points I will receive from remote processors
  std::map<processor_id_type, std::vector<Point>> incoming_qps;
  if (!_qps_cached)
  {
    auto qps_action_functor = [&incoming_qps](processor_id_type pid, const std::vector<Point> & qps)
    {
      // Quadrature points from processor 'pid'
      auto & incoming_qps_from_pid = incoming_qps[pid];
      // Store data for late use
      incoming_qps_from_pid.reserve(incoming_qps_from_pid.size() + qps.size());
      std::copy(qps.begin(), qps.end(), std::back_inserter(incoming_qps_from_pid));
    };

    Parallel::push_parallel_vector_data(comm(), outgoing_qps, qps_action_functor);
  }

  // Cache data
  if (!_qps_cached)
    _cached_qps = incoming_qps;

  for (auto & qps : _cached_qps)
  {
    const processor_id_type pid = qps.first;

    outgoing_evals_ids[pid].resize(qps.second.size(),
                                   std::make_pair(OutOfMeshValue, libMesh::invalid_uint));

    for (unsigned int qp = 0; qp < qps.second.size(); qp++)
    {
      Point qpt = qps.second[qp];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (unsigned int i_from = 0; i_from < _from_problems.size(); i_from++)
      {
        if (local_bboxes[i_from].contains_point(qpt))
        {
          const auto from_global_num =
              _current_direction == TO_MULTIAPP ? 0 : _from_local2global_map[i_from];
          outgoing_evals_ids[pid][qp].first =
              (local_meshfuns[i_from])(_from_transforms[from_global_num]->mapBack(qpt));
          if (_current_direction == FROM_MULTIAPP)
            outgoing_evals_ids[pid][qp].second = from_global_num;
        }
      }
    }
  }

  ////////////////////
  // Gather all of the qp evaluations and pick out the best ones for each qp.
  ////////////////////

  // Values back from remote processors for my local quadrature points
  std::map<processor_id_type, std::vector<std::pair<Real, unsigned int>>> incoming_evals_ids;

  auto evals_action_functor =
      [&incoming_evals_ids](processor_id_type pid,
                            const std::vector<std::pair<Real, unsigned int>> & evals)
  {
    // evals for processor 'pid'
    auto & incoming_evals_ids_for_pid = incoming_evals_ids[pid];
    // Copy evals for late use
    incoming_evals_ids_for_pid.reserve(incoming_evals_ids_for_pid.size() + evals.size());
    std::copy(evals.begin(), evals.end(), std::back_inserter(incoming_evals_ids_for_pid));
  };

  Parallel::push_parallel_vector_data(comm(), outgoing_evals_ids, evals_action_functor);

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

    for (const auto & elem : to_mesh.active_local_element_ptr_range())
    {
      fe->reinit(elem);

      bool element_is_evaled = false;
      std::vector<Real> evals(qrule.n_points(), 0.);

      for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      {
        unsigned int lowest_app_rank = libMesh::invalid_uint;
        for (auto & values_ids : incoming_evals_ids)
        {
          // Current processor id
          const processor_id_type pid = values_ids.first;

          // Ignore the selected processor if the element wasn't found in it's
          // bounding box.
          std::map<std::pair<unsigned int, unsigned int>, unsigned int> & map =
              element_index_map[pid];
          std::pair<unsigned int, unsigned int> key(i_to, elem->id());
          if (map.find(key) == map.end())
            continue;
          unsigned int qp0 = map[key];

          // Ignore the selected processor if it's app has a higher rank than the
          // previously found lowest app rank.
          if (_current_direction == FROM_MULTIAPP)
            if (values_ids.second[qp0 + qp].second >= lowest_app_rank)
              continue;

          // Ignore the selected processor if the qp was actually outside the
          // processor's subapp's mesh.
          if (values_ids.second[qp0 + qp].first == OutOfMeshValue)
            continue;

          // This is the best meshfunction evaluation so far, save it.
          element_is_evaled = true;
          evals[qp] = values_ids.second[qp0 + qp].first;
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

  if (_fixed_meshes)
    _qps_cached = true;

  postExecute();
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

  MooseVariableFEBase & to_var = to_problem.getVariable(
      0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
  System & to_sys = to_var.sys().system();
  NumericVector<Number> * to_solution = to_sys.solution.get();

  for (const auto & node : to_mesh.local_node_ptr_range())
  {
    for (unsigned int comp = 0; comp < node->n_comp(to_sys.number(), to_var.number()); comp++)
    {
      const dof_id_type proj_index = node->dof_number(ls.number(), _proj_var_num, comp);
      const dof_id_type to_index = node->dof_number(to_sys.number(), to_var.number(), comp);
      to_solution->set(to_index, (*ls.solution)(proj_index));
    }
  }
  for (const auto & elem : to_mesh.active_local_element_ptr_range())
    for (unsigned int comp = 0; comp < elem->n_comp(to_sys.number(), to_var.number()); comp++)
    {
      const dof_id_type proj_index = elem->dof_number(ls.number(), _proj_var_num, comp);
      const dof_id_type to_index = elem->dof_number(to_sys.number(), to_var.number(), comp);
      to_solution->set(to_index, (*ls.solution)(proj_index));
    }

  to_solution->close();
  to_sys.update();
}
