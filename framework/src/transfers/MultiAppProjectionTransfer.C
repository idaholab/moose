#include "MultiAppProjectionTransfer.h"
#include "FEProblem.h"
#include "AddVariableAction.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dof_map.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/string_to_enum.h"

#define NOTFOUND -999999

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

MultiAppProjectionTransfer::MultiAppProjectionTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _proj_type(getParam<MooseEnum>("proj_type")),
    _compute_matrix(true)
{
  switch (_direction)
  {
    case TO_MULTIAPP:
      {
        unsigned int n_apps = _multi_app->numGlobalApps();
        _proj_sys.resize(n_apps, NULL);
        for (unsigned int app = 0; app < n_apps; app++)
        {
          if (_multi_app->hasLocalApp(app))
          {
            MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

            FEProblem & to_problem = *_multi_app->appProblem(app);
            FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                           Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
            to_problem.addAuxVariable(_to_var_name, fe_type, NULL);

            EquationSystems & to_es = to_problem.es();
            LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + Utility::enum_to_string<FEFamily>(fe_type.family)
                                                                                           + "-" + Utility::enum_to_string<Order>(fe_type.order));
            _proj_var_num = proj_sys.add_variable("var", fe_type);
            proj_sys.attach_assemble_function(assemble_l2_to);

            _proj_sys[app] = &proj_sys;

            to_problem.hideVariableFromOutput("var");           // hide the auxiliary projection variable

            Moose::swapLibMeshComm(swapped);
          }
        }
      }
      break;

    case FROM_MULTIAPP:
      {
        _proj_sys.resize(1);

        FEProblem & to_problem = *_multi_app->problem();
        FEType fe_type(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                       Utility::string_to_enum<FEFamily>(getParam<MooseEnum>("family")));
        to_problem.addAuxVariable(_to_var_name, fe_type, NULL);

        EquationSystems & to_es = to_problem.es();
        LinearImplicitSystem & proj_sys = to_es.add_system<LinearImplicitSystem>("proj-sys-" + Utility::enum_to_string<FEFamily>(fe_type.family)
                                                                                       + "-" + Utility::enum_to_string<Order>(fe_type.order));
        _proj_var_num = proj_sys.add_variable("var", fe_type);
        proj_sys.attach_assemble_function(assemble_l2_from);

        _proj_sys[0] = &proj_sys;

        to_problem.hideVariableFromOutput("var");           // hide the auxiliary projection variable
      }
      break;
  }
}

MultiAppProjectionTransfer::~MultiAppProjectionTransfer()
{
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

  NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build().release();
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
  AutoPtr<FEBase> fe(FEBase::build(dim, fe_type));
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
}

void
MultiAppProjectionTransfer::assembleL2From(EquationSystems & es, const std::string & system_name)
{
  unsigned int n_apps = _multi_app->numGlobalApps();
  std::vector<NumericVector<Number> *> from_slns(n_apps, NULL);
  std::vector<MeshFunction *> from_fns(n_apps, NULL);
  std::vector<MeshTools::BoundingBox *> from_bbs(n_apps, NULL);

  // get bounding box, mesh function and solution for each subapp
  for (unsigned int i = 0; i < n_apps; i++)
  {
    if (!_multi_app->hasLocalApp(i))
      continue;

    MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

    FEProblem & from_problem = *_multi_app->appProblem(i);
    EquationSystems & from_es = from_problem.es();
    MeshBase & from_mesh = from_es.get_mesh();
    MeshTools::BoundingBox * app_box = new MeshTools::BoundingBox(MeshTools::processor_bounding_box(from_mesh, libMesh::processor_id()));
    from_bbs[i] = app_box;

    MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
    System & from_sys = from_var.sys().system();
    unsigned int from_var_num = from_sys.variable_number(from_var.name());

    NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build().release();
    serialized_from_solution->init(from_sys.n_dofs(), false, SERIAL);
    // Need to pull down a full copy of this vector on every processor so we can get values in parallel
    from_sys.solution->localize(*serialized_from_solution);
    from_slns[i] = serialized_from_solution;

    MeshFunction * from_func = new MeshFunction(from_es, *serialized_from_solution, from_sys.get_dof_map(), from_var_num);
    from_func->init(Trees::ELEMENTS);
    from_func->enable_out_of_mesh_mode(NOTFOUND);
    from_fns[i] = from_func;

    Moose::swapLibMeshComm(swapped);
  }


  const MeshBase& mesh = es.get_mesh();
  const unsigned int dim = mesh.mesh_dimension();

  LinearImplicitSystem & system = es.get_system<LinearImplicitSystem>(system_name);

  FEType fe_type = system.variable_type(0);
  AutoPtr<FEBase> fe(FEBase::build(dim, fe_type));
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
      Real f = 0.;
      for (unsigned int app = 0; app < n_apps; app++)
      {
        Point pt = qpt - _multi_app->position(app);
        if (from_bbs[app]->contains_point(pt))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
          f = (*from_fns[app])(pt);
          Moose::swapLibMeshComm(swapped);
          break;
        }
      }

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

  for (unsigned int i = 0; i < n_apps; i++)
  {
    delete from_fns[i];
    delete from_bbs[i];
    delete from_slns[i];
  }
}


void
MultiAppProjectionTransfer::execute()
{
  Moose::out << "Beginning projection transfer " << _name << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
      toMultiApp();
      break;

    case FROM_MULTIAPP:
      fromMultiApp();
      break;
  }

  Moose::out << "Finished projection transfer " << _name << std::endl;
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
  Moose::out << "Projecting solution" << std::endl;

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
  Moose::out << "Projecting solution" << std::endl;
  projectSolution(*_multi_app->problem(), 0);
}
