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
  checkValidTransferProblemTypes<FEProblemBase, MFEMProblem>();
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
  {
    mooseError("Transfers of non-nodal FEs of between libMesh and MFEM with order higher than "
               "CONSTANT are not supported.");
  }
  else if (is_nodal)
  {
    for (const auto & node : to_mesh.local_node_ptr_range())
    {
      // Skip this node if the variable has no dofs at it.
      if (node->n_dofs(sys_num, var_num) < 1)
        continue;
      outgoing_libmesh_points.push_back(*node);
    }
  }
  else // Elemental, constant monomial
  {
    for (const auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (elem->n_dofs(sys_num, var_num) < 1)
        continue;
      outgoing_libmesh_points.push_back(elem->vertex_average());
    }
  }
}

/// Extract locations from libMesh-based MooseVariable at which projection will take place
void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::projectlibMeshNodalValues(
    libMesh::System & to_sys, const MooseVariableFieldBase & to_var, mfem::Vector & interp_vals)
{
  // Update libMesh solution DoFs with interpolated MFEM values libMesh mesh
  auto var_num = to_var.number();
  auto sys_num = to_sys.number();
  auto & fe_type = to_var.feType();
  bool is_nodal = to_var.isNodal();
  const MeshBase & to_mesh = getActiveToProblem().mesh(_displaced_target_mesh).getMesh();

  unsigned int mfem_point_index = 0;
  if (fe_type.order > CONSTANT && !is_nodal)
  {
    mooseError("Transfers of non-nodal FEs of between libMesh and MFEM with order higher than "
               "CONSTANT are not supported.");
  }
  else if (is_nodal)
  {
    for (const auto & node : to_mesh.local_node_ptr_range())
    {
      // Skip this node if the variable has no dofs at it.
      if (node->n_dofs(sys_num, var_num) < 1)
        continue;
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
    for (const auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (elem->n_dofs(sys_num, var_num) < 1)
        continue;
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
MultiAppMFEMTolibMeshShapeEvaluationTransfer::transferVariables()
{
  // Send from MFEM problem to libMesh problem
  for (unsigned var_index = 0; var_index < numToVar(); ++var_index)
  {
    /// Store all target libMesh variables in a vector for convenience
    std::vector<MooseVariableFieldBase *> to_variables;
    if (_to_problems.size())
    {
      to_variables.resize(numToVar());
      for (const auto i_var : index_range(to_variables))
        to_variables[i_var] = &_to_problems[0]->getVariable(0,
                                                            getToVarName(i_var),
                                                            Moose::VarKindType::VAR_ANY,
                                                            Moose::VarFieldType::VAR_FIELD_ANY);
    }

    for (const auto problem_id : index_range(_to_problems))
    {
      // Declare aliases for convenience
      setActiveToProblem(*_to_problems[problem_id]);
      const MooseVariableFieldBase & to_var(*to_variables[var_index]);
      const auto & var_name = getToVarName(var_index);
      auto & es = getActiveToProblem().es();
      System & to_sys = *find_sys(es, var_name);
      // Extract set of target points in libMesh mesh to perform interpolation of MFEM variable at
      std::vector<Point> outgoing_libmesh_points;
      extractlibMeshNodePositions(to_sys, to_var, outgoing_libmesh_points);
      const MeshBase & to_mesh = getActiveToProblem().mesh(_displaced_target_mesh).getMesh();

      // Perform interpolation of MFEM variable
      const mfem::Ordering::Type ordering = mfem::Ordering::byVDIM;
      mfem::Vector outgoing_mfem_points = Moose::MFEM::libMeshPointsToMFEMVector(
          outgoing_libmesh_points, to_mesh.mesh_dimension(), ordering);
      mfem::Vector interp_vals;
      auto & from_var =
          getActiveFromProblem().getProblemData().gridfunctions.GetRef(getFromVarName(var_index));
      from_var.ParFESpace()->GetParMesh()->EnsureNodes();
      _mfem_interpolator.SetDefaultInterpolationValue(
          std::numeric_limits<mfem::real_t>::infinity());
      _mfem_interpolator.Interpolate(*from_var.ParFESpace()->GetParMesh(),
                                     outgoing_mfem_points,
                                     from_var,
                                     interp_vals,
                                     ordering);

      // Project interpolated values at destination nodes onto destination variables to set DoFs
      projectlibMeshNodalValues(to_sys, to_var, interp_vals);
    }
  }
}

#endif
