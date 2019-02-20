// MOOSE includes
#include "LaplaceProjectionTransfer.h"
#include "FEProblem.h"
#include "AddVariableAction.h"
#include "MooseMesh.h"
#include "AuxiliarySystem.h"
#include "MooseVariable.h"
// libMesh includes
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dof_map.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/sparse_matrix.h"

void
assemble_laplace_proj(EquationSystems & es, const std::string & system_name)
{
  LaplaceProjectionTransfer * transfer = es.parameters.get<LaplaceProjectionTransfer *>("transfer");
  transfer->assembleLaplaceProjection(es, system_name);
}

registerMooseObject("THMApp", LaplaceProjectionTransfer);

template <>
InputParameters
validParams<LaplaceProjectionTransfer>()
{
  InputParameters params = validParams<Transfer>();
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("fixed_meshes",
                        false,
                        "Set to true when the meshes are not changing (ie, "
                        "no movement or adaptivity).  This will cache some "
                        "information to speed up the transfer.");

  return params;
}

LaplaceProjectionTransfer::LaplaceProjectionTransfer(const InputParameters & parameters)
  : Transfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _fixed_meshes(getParam<bool>("fixed_meshes"))
{
}

void
LaplaceProjectionTransfer::initialSetup()
{
  EquationSystems & es = _fe_problem.es();

  // Add the projection system.
  MooseVariable & from_var = _fe_problem.getStandardVariable(0, _from_var_name);
  FEType fe_type = _fe_problem.getStandardVariable(0, _to_var_name).feType();
  LinearImplicitSystem & proj_sys =
      es.add_system<LinearImplicitSystem>("laplace-proj-sys-" + name());
  _proj_var_num = proj_sys.add_variable("var", fe_type, &from_var.activeSubdomains());
  proj_sys.attach_assemble_function(assemble_laplace_proj);
  _proj_sys = &proj_sys;

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
  es.reinit();
}

void
LaplaceProjectionTransfer::assembleLaplaceProjection(EquationSystems & es,
                                                     const std::string & system_name)
{
  // Get the system and mesh from the input arguments.
  LinearImplicitSystem & system = es.get_system<LinearImplicitSystem>(system_name);
  MeshBase & to_mesh = es.get_mesh();

  // Setup system vectors and matrices.
  FEType fe_type = system.variable_type(0);
  std::unique_ptr<FEBase> fe(FEBase::build(to_mesh.mesh_dimension(), fe_type));
  QGauss qrule(to_mesh.mesh_dimension(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  const DofMap & dof_map = system.get_dof_map();
  DenseMatrix<Number> Ke;
  DenseVector<Number> Fe;
  unsigned int nqp = qrule.n_points();
  std::vector<dof_id_type> dof_indices;
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<std::vector<Real>> & phi = fe->get_phi();
  const std::vector<std::vector<RealGradient>> & dphi = fe->get_dphi();

  std::unique_ptr<FEBase> fe_face(FEBase::build(to_mesh.mesh_dimension(), fe_type));
  QGauss qrule_face(to_mesh.mesh_dimension() - 1, fe_type.default_quadrature_order());
  fe_face->attach_quadrature_rule(&qrule_face);
  unsigned int from_var_num = _fe_problem
                                  .getVariable(0,
                                               _from_var_name,
                                               Moose::VarKindType::VAR_ANY,
                                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                                  .number();
  const DofMap & from_dof_map = _fe_problem.getAuxiliarySystem().dofMap();
  std::vector<dof_id_type> from_dof_indices;
  // FIXME: remove the need of calling getAuxiliarySystem()
  const NumericVector<Real> & from_current_solution =
      *_fe_problem.getAuxiliarySystem().currentSolution();
  std::vector<RealGradient> from_var_grad(nqp);

  const MeshBase::const_element_iterator end_el = to_mesh.active_local_elements_end();
  for (MeshBase::const_element_iterator el = to_mesh.active_local_elements_begin(); el != end_el;
       ++el)
  {
    const Elem * elem = *el;

    from_dof_map.dof_indices(elem, from_dof_indices, from_var_num);
    if (from_dof_indices.size() > 0)
    {
      fe->reinit(elem);
      // compute values of the source FE variable
      for (unsigned int i = 0; i < nqp; ++i)
        from_var_grad[i] = 0;

      for (unsigned int i = 0; i < phi.size(); i++)
      {
        unsigned int idx = from_dof_indices[i];
        Real soln_local = from_current_solution(idx);

        for (unsigned int qp = 0; qp < nqp; qp++)
          from_var_grad[qp].add_scaled(dphi[i][qp], soln_local);
      }

      dof_map.dof_indices(elem, dof_indices);

      Ke.resize(dof_indices.size(), dof_indices.size());
      Fe.resize(dof_indices.size());

      for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      {
        // Now compute the element matrix and RHS contributions.
        for (unsigned int i = 0; i < phi.size(); i++)
        {
          // RHS
          Fe(i) -= JxW[qp] * (from_var_grad[qp] * dphi[i][qp]);

          for (unsigned int j = 0; j < phi.size(); j++)
          {
            // The matrix contribution
            Ke(i, j) += JxW[qp] * (phi[i][qp] * phi[j][qp]);
          }
        }
      }

      // side
      for (unsigned int side = 0; side < elem->n_sides(); side++)
      {
        if (elem->neighbor_ptr(side) == libmesh_nullptr)
        {
          const std::vector<std::vector<Real>> & phi_face = fe_face->get_phi();
          const std::vector<std::vector<RealGradient>> & dphi_face = fe_face->get_dphi();
          const std::vector<Real> & JxW_face = fe_face->get_JxW();
          const std::vector<Point> & normals = fe_face->get_normals();

          fe_face->reinit(elem, side);

          unsigned int nqp_face = qrule_face.n_points();

          // compute values of the source FE variable
          for (unsigned int i = 0; i < nqp_face; ++i)
            from_var_grad[i] = 0;

          for (unsigned int i = 0; i < phi.size(); i++)
          {
            unsigned int idx = from_dof_indices[i];
            Real soln_local = from_current_solution(idx);

            for (unsigned int qp = 0; qp < nqp_face; qp++)
              from_var_grad[qp].add_scaled(dphi_face[i][qp], soln_local);
          }

          for (unsigned int qp = 0; qp < nqp_face; qp++)
          {
            for (unsigned int i = 0; i < phi_face.size(); i++)
              Fe(i) += JxW_face[qp] * from_var_grad[qp] * normals[qp] * phi_face[i][qp];
          }
        }
      } // sides

      dof_map.constrain_element_matrix_and_vector(Ke, Fe, dof_indices);

      system.matrix->add_matrix(Ke, dof_indices);
      system.rhs->add_vector(Fe, dof_indices);
    }
  }
}

void
LaplaceProjectionTransfer::execute()
{
  projectSolution();
}

void
LaplaceProjectionTransfer::projectSolution()
{
  FEProblemBase & to_problem = _fe_problem;
  EquationSystems & proj_es = to_problem.es();
  LinearImplicitSystem & ls = *_proj_sys;
  // activate the current transfer
  proj_es.parameters.set<LaplaceProjectionTransfer *>("transfer") = this;

  // TODO: specify solver params in an input file
  // solver tolerance
  Real tol = proj_es.parameters.get<Real>("linear solver tolerance");
  proj_es.parameters.set<Real>("linear solver tolerance") = 1e-10; // set our tolerance
  // solve it
  ls.solve();
  proj_es.parameters.set<Real>("linear solver tolerance") = tol; // restore the original tolerance

  // copy projected solution into target es
  MeshBase & to_mesh = proj_es.get_mesh();

  MooseVariable & to_var = to_problem.getStandardVariable(0, _to_var_name);
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

  to_solution->close();
  to_sys.update();
}
