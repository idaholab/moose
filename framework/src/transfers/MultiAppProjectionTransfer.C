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
#include "MultiAppProjectionTransfer.h"
#include "FEProblem.h"
#include "AddVariableAction.h"
#include "MooseError.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dof_map.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/parallel_algebra.h"


void assemble_l2_from(EquationSystems & es, const std::string & system_name)
{
  MultiAppProjectionTransfer * transfer = es.parameters.get<MultiAppProjectionTransfer *>("transfer");
  transfer->assembleL2From(es, system_name);
}

void assemble_l2_to(EquationSystems & es, const std::string & system_name)
{
  MultiAppProjectionTransfer * transfer = es.parameters.get<MultiAppProjectionTransfer *>("transfer");
  transfer->assembleL2To(es, system_name);
}


template<>
InputParameters validParams<MultiAppProjectionTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");

  MooseEnum proj_type("l2", "l2");
  params.addParam<MooseEnum>("proj_type", proj_type, "The type of the projection.");

  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable (additional orders not listed are allowed)");

  return params;
}

MultiAppProjectionTransfer::MultiAppProjectionTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _proj_type(getParam<MooseEnum>("proj_type")),
    _compute_matrix(true)
{
}

MultiAppProjectionTransfer::~MultiAppProjectionTransfer()
{
}

void
MultiAppProjectionTransfer::initialSetup()
{
  switch (_direction)
  {
    case TO_MULTIAPP:
      {
        unsigned int n_apps = _multi_app->numGlobalApps();
        _proj_sys.resize(n_apps, NULL);

        // Keep track of which EquationSystems just had new Systems
        // added to them
        std::set<EquationSystems *> augmented_es;

        for (unsigned int app = 0; app < n_apps; app++)
        {
          if (_multi_app->hasLocalApp(app))
          {
            MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

            FEProblem & to_problem = *_multi_app->appProblem(app);
            FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                           Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
            //to_problem.addAuxVariable(_to_var_name, fe_type, NULL);

            EquationSystems & to_es = to_problem.es();
            LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + Utility::enum_to_string<FEFamily>(fe_type.family)
                                                                                           + "-" + Utility::enum_to_string<Order>(fe_type.order)
                                                                                           + "-" + name());
            _proj_var_num = proj_sys.add_variable("var", fe_type);
            proj_sys.attach_assemble_function(assemble_l2_to);

            _proj_sys[app] = &proj_sys;

            // We'll defer to_es.reinit() so we don't do it multiple
            // times even if we add multiple new systems
            augmented_es.insert(&to_es);

            //to_problem.hideVariableFromOutput("var");           // hide the auxiliary projection variable

            Moose::swapLibMeshComm(swapped);
          }
        }

        // Make sure all new systems are initialized.
        for (std::set<EquationSystems *>::iterator es_iter =
             augmented_es.begin();
             es_iter != augmented_es.end(); ++es_iter)
          {
            EquationSystems *es = *es_iter;
            es->reinit();
          }
      }
      break;

    case FROM_MULTIAPP:
      {
        _proj_sys.resize(1);

        FEProblem & to_problem = *_multi_app->problem();
        FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                       Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
        //to_problem.addAuxVariable(_to_var_name, fe_type, NULL);

        EquationSystems & to_es = to_problem.es();
        LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + Utility::enum_to_string<FEFamily>(fe_type.family)
                                                                                       + "-" + Utility::enum_to_string<Order>(fe_type.order)
                                                                                           + "-" + name());
        _proj_var_num = proj_sys.add_variable("var", fe_type);
        proj_sys.attach_assemble_function(assemble_l2_from);

        _proj_sys[0] = &proj_sys;

        // to_problem.hideVariableFromOutput("var");           // hide the auxiliary projection variable
        to_es.reinit();
      }
      break;
  }
}

void
MultiAppProjectionTransfer::assembleL2To(EquationSystems & es, const std::string & system_name)
{
  unsigned int app = es.parameters.get<unsigned int>("app");

  FEProblem & from_problem = *_multi_app->problem();
  EquationSystems & from_es = from_problem.es();

  MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
  System & from_sys = from_var.sys().system();
  unsigned int from_var_num = from_sys.variable_number(from_var.name());

  NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build(from_sys.comm()).release();
  serialized_from_solution->init(from_sys.n_dofs(), false, SERIAL);
  // Need to pull down a full copy of this vector on every processor so we can get values in parallel
  from_sys.solution->localize(*serialized_from_solution);

  MeshFunction from_func(from_es, *serialized_from_solution, from_sys.get_dof_map(), from_var_num);
  from_func.init(Trees::ELEMENTS);
  from_func.enable_out_of_mesh_mode(0.);


  const MeshBase& mesh = es.get_mesh();
  const unsigned int dim = mesh.mesh_dimension();

  LinearImplicitSystem & system = es.get_system<LinearImplicitSystem>(system_name);

  FEType fe_type = system.variable_type(0);
  UniquePtr<FEBase> fe(FEBase::build(dim, fe_type));
  QGauss qrule(dim, fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<Real> > & phi = fe->get_phi();
  const std::vector<Point> & xyz = fe->get_xyz();

  const DofMap& dof_map = system.get_dof_map();
  DenseMatrix<Number> Ke;
  DenseVector<Number> Fe;
  std::vector<dof_id_type> dof_indices;

  MeshBase::const_element_iterator       el     = mesh.active_local_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_local_elements_end();
  for ( ; el != end_el; ++el)
  {
    const Elem* elem = *el;

    fe->reinit (elem);

    dof_map.dof_indices (elem, dof_indices);
    Ke.resize (dof_indices.size(), dof_indices.size());
    Fe.resize (dof_indices.size());

    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
    {
      Point qpt = xyz[qp];
      Point pt = qpt + _multi_app->position(app);
      Real f = from_func(pt);

      // Now compute the element matrix and RHS contributions.
      for (unsigned int i=0; i<phi.size(); i++)
      {
        // RHS
        Fe(i) += JxW[qp] * (f * phi[i][qp]);

        if (_compute_matrix)
          for (unsigned int j = 0; j < phi.size(); j++)
          {
            // The matrix contribution
            Ke(i,j) += JxW[qp] * (phi[i][qp] * phi[j][qp]);
          }
      }
      dof_map.constrain_element_matrix_and_vector(Ke, Fe, dof_indices);

      if (_compute_matrix)
        system.matrix->add_matrix(Ke, dof_indices);
      system.rhs->add_vector(Fe, dof_indices);
    }
  }
  delete serialized_from_solution;
}

void
MultiAppProjectionTransfer::assembleL2From(EquationSystems & es, const std::string & system_name)
{
  /********************
  First step, get the bounding boxes and mesh functions for all the sub apps on
  this processor.
  ********************/
  const unsigned int n_global_apps = _multi_app->numGlobalApps();
  const unsigned int n_local_apps = _multi_app->numLocalApps();
  std::vector<NumericVector<Number> *> from_slns(n_local_apps, NULL);
  std::vector<MeshFunction *> from_fns(n_local_apps, NULL);
  std::vector<std::pair<Point, Point> > bb_points(n_local_apps);
  std::vector<MeshTools::BoundingBox> from_bbs(n_local_apps);

  unsigned int i_local = 0;
  for (unsigned int i_global = 0; i_global < n_global_apps; i_global++)
  {
    if (!_multi_app->hasLocalApp(i_global))
      continue;

    MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

    // Get the bounding box.
    FEProblem & from_problem = *_multi_app->appProblem(i_global);
    EquationSystems & from_es = from_problem.es();
    MeshBase & from_mesh = from_es.get_mesh();
    MeshTools::BoundingBox app_box = MeshTools::BoundingBox(MeshTools::processor_bounding_box(from_mesh, from_mesh.processor_id()));

    // Cast the bounding box into a pair of points to simplify MPI
    // communication.  Translate the bounding box to the app's position.
    bb_points[i_local] = static_cast<std::pair<Point, Point> >(app_box);
    bb_points[i_local].first = bb_points[i_local].first + _multi_app->position(i_global);
    bb_points[i_local].second = bb_points[i_local].second + _multi_app->position(i_global);
    //from_bbs[i_local] = app_box;
    from_bbs[i_local] = static_cast<MeshTools::BoundingBox>(bb_points[i_local]);

    // Get a serialized copy of the subapp's solution vector.
    MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build(from_sys.comm()).release();
    serialized_from_solution->init(from_sys.n_dofs(), false, SERIAL);
    from_sys.solution->localize(*serialized_from_solution);
    from_slns[i_local] = serialized_from_solution;

    // Get the subapp's mesh function.
    MeshFunction * from_func = new MeshFunction(from_es, *serialized_from_solution, from_sys.get_dof_map(), from_var_num);
    from_func->init(Trees::ELEMENTS);
    from_func->enable_out_of_mesh_mode(OutOfMeshValue);
    from_fns[i_local] = from_func;

    Moose::swapLibMeshComm(swapped);

    i_local++;
  }

  /********************
  Next, serialize the bounding boxes, and keep track of how many apps (i.e. how
  many bounding boxes) each processor has.
  ********************/
  std::vector<unsigned int> apps_per_proc(1, n_local_apps);

  _communicator.allgather(bb_points);
  _communicator.allgather(apps_per_proc, true);

  if (apps_per_proc.size() != n_processors())
    mooseError("Transfer failed to gather data from all processors.");

  unsigned int n_sources = 0;
  for (unsigned int i=0; i<n_processors(); i++)
  {
    n_sources += apps_per_proc[i];
  }

  if (bb_points.size() != n_sources)
    mooseError("Transfer failed to gather data from all processors.");

  std::vector<MeshTools::BoundingBox> bboxes(n_sources);
  for (unsigned int i=0; i<n_sources; i++)
  {
    bboxes[i] = static_cast<MeshTools::BoundingBox>(bb_points[i]);
  }

  /********************
  Now, check all the elements local to this processor and see if they overlap
  with any of the bounding boxes from other processors.  Keep track of which
  elements overlap with which processors.  Build vectors of quadrature points to
  send to other processors for mesh function evaluations.
  ********************/
  const MeshBase& mesh = es.get_mesh();
  const unsigned int dim = mesh.mesh_dimension();

  LinearImplicitSystem & system = es.get_system<LinearImplicitSystem>(system_name);

  FEType fe_type = system.variable_type(0);
  UniquePtr<FEBase> fe(FEBase::build(dim, fe_type));
  QGauss qrule(dim, fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  const std::vector<Point> & xyz = fe->get_xyz();
  std::vector<std::vector<Point> > outgoing_qps(n_processors());
  std::vector< std::unordered_map<unsigned int, unsigned int> > element_index_map(n_processors());

  for (unsigned int i_proc = 0, app0 = 0;
       i_proc < n_processors();
       i_proc++, app0 += apps_per_proc[i_proc])
  {
    MeshBase::const_element_iterator       el     = mesh.active_local_elements_begin();
    const MeshBase::const_element_iterator end_el = mesh.active_local_elements_end();
    for ( ; el != end_el; ++el)
    {
      const Elem* elem = *el;
      fe->reinit (elem);

      bool qp_hit = false;
      for (unsigned int i_app = 0;
           i_app < apps_per_proc[i_proc] && ! qp_hit; i_app++)
      {
        for (unsigned int qp = 0;
             qp < qrule.n_points() && ! qp_hit; qp ++)
        {
          Point qpt = xyz[qp];
          if (bboxes[app0 + i_app].contains_point(qpt))
            qp_hit = true;
        }
      }

      if (qp_hit)
      {
        // This processor's bounding box contains at least one qudrature point
        // from this element.
        element_index_map[i_proc][elem->id()] = outgoing_qps[i_proc].size();
        for (unsigned int qp = 0; qp < qrule.n_points(); qp ++)
        {
          Point qpt = xyz[qp];
          outgoing_qps[i_proc].push_back(qpt);
        }
      }
    }
  }

  /********************
  Request quadrature point evaluations from other processors and handle requests
  sent to this processor.
  ********************/
  std::vector<std::vector<Real> > incoming_evals(n_processors());
  std::vector<std::vector<unsigned int> > incoming_app_ids(n_processors());
  for (unsigned int i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;

    //_console << "Processor " << processor_id() << " sending " << outgoing_qps[i_proc].size() << " points to processor " << i_proc << std::endl;
    _communicator.send(i_proc, outgoing_qps[i_proc]);
  }

  for (unsigned int i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    std::vector<Point> incoming_qps;
    if (i_proc == processor_id())
      incoming_qps = outgoing_qps[i_proc];
    else
      _communicator.receive(i_proc, incoming_qps);

    std::vector<Real> outgoing_evals(incoming_qps.size(), OutOfMeshValue);
    std::vector<unsigned int> outgoing_ids(incoming_qps.size(), -1); // -1 = largest unsigned int
    for (unsigned int qp = 0; qp < incoming_qps.size(); qp++)
    {
      Point qpt = incoming_qps[qp];

      // Loop until we've found the lowest-ranked app that actually contains
      // the quadrature point.
      for (unsigned int i_global = 0, i_local = 0;
           i_global < n_global_apps && outgoing_evals[qp] == OutOfMeshValue;
           i_global++)
      {
        if (!_multi_app->hasLocalApp(i_global))
          continue;
        if (from_bbs[i_local].contains_point(qpt))
        {
          outgoing_evals[qp] = (* from_fns[i_local])(qpt - _multi_app->position(i_global));
          outgoing_ids[qp] = i_global;
        }
        i_local ++;
      }
    }

    if (i_proc == processor_id())
    {
      incoming_evals[i_proc] = outgoing_evals;
      incoming_app_ids[i_proc] = outgoing_ids;
    }
    else
    {
      _communicator.send(i_proc, outgoing_evals);
      _communicator.send(i_proc, outgoing_ids);
    }
  }

  /********************
  Gather all of the qp evaluations, find the best one for each qp, and define
  the system.
  ********************/
  for (unsigned int i_proc = 0; i_proc < n_processors(); i_proc++)
  {
    if (i_proc == processor_id())
      continue;

    _communicator.receive(i_proc, incoming_evals[i_proc]);
    _communicator.receive(i_proc, incoming_app_ids[i_proc]);
  }

  const DofMap& dof_map = system.get_dof_map();
  DenseMatrix<Number> Ke;
  DenseVector<Number> Fe;
  std::vector<dof_id_type> dof_indices;
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<Real> > & phi = fe->get_phi();

  MeshBase::const_element_iterator       el     = mesh.active_local_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_local_elements_end();
  for ( ; el != end_el; ++el)
  {
    const Elem* elem = *el;
    fe->reinit (elem);

    dof_map.dof_indices (elem, dof_indices);
    Ke.resize (dof_indices.size(), dof_indices.size());
    Fe.resize (dof_indices.size());

    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
    {
      Point qpt = xyz[qp];

      unsigned int lowest_app_rank = -1; // -1 = largest unsigned int
      Real meshfun_eval = 0.;
      for (unsigned int i_proc = 0; i_proc < n_processors(); i_proc++)
      {
        std::unordered_map<unsigned int, unsigned int> & map = element_index_map[i_proc];
        // Ignore this processor if the element wasn't found in it's bounding
        // box.
        if (map.find(elem->id()) == map.end())
          continue;
        unsigned int qp0 = map[elem->id()];

        // Ignore this processor if it's app has a higher rank than the
        // previously found lowest app rank.
        if (incoming_app_ids[i_proc][qp0 + qp] >= lowest_app_rank)
          continue;

        // Ignore this processor if the qp was actually outside the processor's
        // mesh.
        if (incoming_evals[i_proc][qp0 + qp] == OutOfMeshValue)
          continue;

        meshfun_eval = incoming_evals[i_proc][qp0 + qp];
      }

      // Now compute the element matrix and RHS contributions.
      for (unsigned int i=0; i<phi.size(); i++)
      {
        // RHS
        Fe(i) += JxW[qp] * (meshfun_eval * phi[i][qp]);

        if (_compute_matrix)
          for (unsigned int j = 0; j < phi.size(); j++)
          {
            // The matrix contribution
            Ke(i,j) += JxW[qp] * (phi[i][qp] * phi[j][qp]);
          }
      }
      dof_map.constrain_element_matrix_and_vector(Ke, Fe, dof_indices);

      if (_compute_matrix)
        system.matrix->add_matrix(Ke, dof_indices);
      system.rhs->add_vector(Fe, dof_indices);
    }
  }

  for (unsigned int i = 0; i < n_local_apps; i++)
  {
    delete from_fns[i];
    delete from_slns[i];
  }
}


void
MultiAppProjectionTransfer::execute()
{
  _console << "Beginning projection transfer " << name() << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
      toMultiApp();
      break;

    case FROM_MULTIAPP:
      fromMultiApp();
      break;
  }

  _console << "Finished projection transfer " << name() << std::endl;
}

void
MultiAppProjectionTransfer::projectSolution(FEProblem & to_problem, unsigned int app)
{
  EquationSystems & proj_es = to_problem.es();
  LinearImplicitSystem & ls = *_proj_sys[app];
  // activate the current transfer
  proj_es.parameters.set<MultiAppProjectionTransfer *>("transfer") = this;
  proj_es.parameters.set<unsigned int>("app") = app;

  // TODO: specify solver params in an input file
  // solver tolerance
  Real tol = proj_es.parameters.get<Real>("linear solver tolerance");
  proj_es.parameters.set<Real>("linear solver tolerance") = 1e-10;      // set our tolerance
  // solve it
  ls.solve();
  proj_es.parameters.set<Real>("linear solver tolerance") = tol;        // restore the original tolerance

  // copy projected solution into target es
  MeshBase & to_mesh = proj_es.get_mesh();

  MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
  System & to_sys = to_var.sys().system();
  NumericVector<Number> * to_solution = to_sys.solution.get();

  {
    MeshBase::const_node_iterator it = to_mesh.local_nodes_begin();
    const MeshBase::const_node_iterator end_it = to_mesh.local_nodes_end();
    for ( ; it != end_it; ++it)
    {
      const Node * node = *it;
      if (node->n_comp(to_sys.number(), to_var.number()) > 0)
      {
        const dof_id_type proj_index = node->dof_number(ls.number(), _proj_var_num, 0);
        const dof_id_type to_index = node->dof_number(to_sys.number(), to_var.number(), 0);
        to_solution->set(to_index, (*ls.solution)(proj_index));
      }
    }
  }
  {
    MeshBase::const_element_iterator it = to_mesh.active_local_elements_begin();
    const MeshBase::const_element_iterator end_it = to_mesh.active_local_elements_end();
    for ( ; it != end_it; ++it)
    {
      const Elem * elem = *it;
      if (elem->n_comp(to_sys.number(), to_var.number()) > 0)
      {
        const dof_id_type proj_index = elem->dof_number(ls.number(), _proj_var_num, 0);
        const dof_id_type to_index = elem->dof_number(to_sys.number(), to_var.number(), 0);
        to_solution->set(to_index, (*ls.solution)(proj_index));
      }
    }
  }

  to_solution->close();
  to_sys.update();
}

void
MultiAppProjectionTransfer::toMultiApp()
{
  _console << "Projecting solution" << std::endl;

  for (unsigned int app = 0; app < _multi_app->numGlobalApps(); app++)
  {
    if (_multi_app->hasLocalApp(app))
    {
      MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
      projectSolution(*_multi_app->appProblem(app), app);
      Moose::swapLibMeshComm(swapped);
    }
  }
}

void
MultiAppProjectionTransfer::fromMultiApp()
{
  _console << "Projecting solution" << std::endl;
  projectSolution(*_multi_app->problem(), 0);
}

