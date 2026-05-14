//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMTolibMeshShapeEvaluationTransfer.h"
#include "MFEMVectorUtils.h"
#include "libmesh/mesh_function.h"

registerMooseObject("MooseApp", MultiAppMFEMTolibMeshShapeEvaluationTransfer);

InputParameters
MultiAppMFEMTolibMeshShapeEvaluationTransfer::validParams()
{
  InputParameters params = MFEMMultiAppTransfer::validParams();
  params.addClassDescription("Transfers variable values from an MFEM based application to a "
                             "libMesh application, using shape function evaluations.");
  return params;
}

MultiAppMFEMTolibMeshShapeEvaluationTransfer::MultiAppMFEMTolibMeshShapeEvaluationTransfer(
    InputParameters const & params)
  : MFEMMultiAppTransfer(params), _mfem_interpolator(this->comm().get())
{
  checkValidTransferProblemTypes<Moose::FEBackend::LibMesh, Moose::FEBackend::MFEM>();
}

MFEMProblem &
MultiAppMFEMTolibMeshShapeEvaluationTransfer::getActiveFromProblem()
{
  return static_cast<MFEMProblem &>(MFEMMultiAppTransfer::getActiveFromProblem());
}

/// Extract locations from libMesh-based MooseVariable at which projection will take place
void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::extractlibMeshNodePositions(
    libMesh::System & to_sys,
    const MooseVariableFieldBase & to_var,
    std::vector<Point> & outgoing_libmesh_points)
{
  // libMesh mesh
  const MeshBase & to_mesh = getActiveToProblem().mesh(_displaced_target_mesh).getMesh();
  auto var_num = to_var.number();
  auto sys_num = to_sys.number();
  auto & fe_type = to_var.feType();
  bool is_nodal = to_var.isNodal();

  // Populate set of points
  if (fe_type.order > CONSTANT && !is_nodal)
    mooseError("Transfers of non-nodal FEs from MFEM to libMesh with order higher than "
               "CONSTANT are not supported.");
  else if (is_nodal)
  {
    // Consider nodes the variable has dofs at.
    for (const auto & node : to_mesh.local_node_ptr_range())
      if (node->n_dofs(sys_num, var_num))
        outgoing_libmesh_points.push_back(*node);
  }
  else // Elemental, constant monomial
  {
    // Consider elements the variable has dofs at.
    for (const auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      if (elem->n_dofs(sys_num, var_num))
        outgoing_libmesh_points.push_back(elem->vertex_average());
  }
}

/// Project interpolated values to set DoFs of libMesh-based MooseVariable
void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::projectlibMeshNodalValues(
    libMesh::System & to_sys, const MooseVariableFieldBase & to_var, mfem::Vector & interp_vals)
{
  // libMesh mesh
  const MeshBase & to_mesh = getActiveToProblem().mesh(_displaced_target_mesh).getMesh();
  auto var_num = to_var.number();
  auto sys_num = to_sys.number();
  auto & fe_type = to_var.feType();
  bool is_nodal = to_var.isNodal();

  unsigned int mfem_point_index = 0;
  if (fe_type.order > CONSTANT && !is_nodal)
    mooseError("Transfers of non-nodal FEs from MFEM to libMesh with order higher than "
               "CONSTANT are not supported.");
  else if (is_nodal)
  {
    // Consider nodes the variable has dofs at.
    for (const auto & node : to_mesh.local_node_ptr_range())
      if (node->n_dofs(sys_num, var_num))
      {
        const auto dof_object_id = node->id();
        const DofObject * dof_object = to_mesh.node_ptr(dof_object_id);
        const auto dof = dof_object->dof_number(sys_num, var_num, 0);
        const auto val = interp_vals[mfem_point_index];
        to_sys.solution->set(dof, val);
        mfem_point_index++;
      }
  }
  else // Elemental, constant monomial
  {
    // Consider elements the variable has dofs at.
    for (const auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      if (elem->n_dofs(sys_num, var_num))
      {
        const auto dof_object_id = elem->id();
        const DofObject * dof_object = to_mesh.elem_ptr(dof_object_id);
        const auto dof = dof_object->dof_number(sys_num, var_num, 0);
        const auto val = interp_vals[mfem_point_index];
        to_sys.solution->set(dof, val);
        mfem_point_index++;
      }
  }
  to_sys.solution->close();
  // Sync local solutions
  to_sys.update();
}

void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::transferVariables(bool is_target_local)
{
  // Send from MFEM problem to libMesh problem
  for (const auto var_index : make_range(numToVar()))
  {
    // Declare source variable alias and construct vectors to store interpolation points and vals
    auto & from_var = *getActiveFromProblem().getGridFunction(getFromVarName(var_index));
    if (from_var.VectorDim() > 1)
      mooseError("MultiAppMFEMTolibMeshShapeEvaluationTransfer does not support transfers of "
                 "vector variables from MFEM to libMesh-based subapps");
    from_var.ParFESpace()->GetParMesh()->EnsureNodes();
    // Ordering of interpolated MFEM output, chosen to match libMesh ordering
    const mfem::Ordering::Type ordering = mfem::Ordering::byVDIM;
    mfem::Vector interp_vals;
    mfem::Vector outgoing_mfem_points;

    // If target libMesh app is on the current rank, update the set of interpolation
    // points with the libMesh variable's set
    if (is_target_local)
    {
      // Declare aliases for convenience
      FEProblemBase & to_problem = getActiveToProblem();
      const MooseVariableFieldBase & to_var(
          to_problem.getVariable(0,
                                 getToVarName(var_index),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY));
      const auto & to_var_name = getToVarName(var_index);
      auto & es = getlibMeshEquationSystem(to_problem, _displaced_target_mesh);
      System & to_sys = *find_sys(es, to_var_name);
      // Extract set of target points in libMesh mesh to perform interpolation of MFEM variable at
      std::vector<Point> outgoing_libmesh_points;
      extractlibMeshNodePositions(to_sys, to_var, outgoing_libmesh_points);
      const MeshBase & to_mesh = to_problem.mesh(_displaced_target_mesh).getMesh();
      // Perform interpolation of MFEM variable
      for (auto & point : outgoing_libmesh_points)
        point = mapPointToActiveSourceFrame(point);
      outgoing_mfem_points = Moose::MFEM::libMeshPointsToMFEMVector(
          outgoing_libmesh_points, to_mesh.mesh_dimension(), ordering);
    }

    // MFEM interpolation using FindPointsGSLib::Interpolate must be over all ranks
    // of the MFEM variable's MPI communicator
    _mfem_interpolator.SetDefaultInterpolationValue(getMFEMOutOfMeshValue());
    _mfem_interpolator.Interpolate(*from_var.ParFESpace()->GetParMesh(),
                                   outgoing_mfem_points,
                                   from_var,
                                   interp_vals,
                                   ordering);

    // If target libMesh variable exists on the current rank, update the set of local
    // DoFs of the target libMesh variable using the interpolated MFEM values
    if (is_target_local)
    {
      // Declare aliases for convenience
      FEProblemBase & to_problem = getActiveToProblem();
      const MooseVariableFieldBase & to_var(
          to_problem.getVariable(0,
                                 getToVarName(var_index),
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_ANY));
      const auto & to_var_name = getToVarName(var_index);
      // Get undisplaced system for writing transferred variable to
      auto & out_es = getlibMeshEquationSystem(to_problem, false);
      auto & out_sys = *find_sys(out_es, to_var_name);

      // Project interpolated values at destination nodes onto destination variables to set DoFs
      projectlibMeshNodalValues(out_sys, to_var, interp_vals);
    }
  }
}

#endif
