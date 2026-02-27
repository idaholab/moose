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

void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::transferVariables()
{
  // Send from MFEM problem to libMesh problem
  auto & mfem_mesh = getActiveFromProblem().mesh().getMFEMParMesh();
  mfem_mesh.EnsureNodes();
  _mfem_interpolator.Setup(mfem_mesh);
  for (unsigned v = 0; v < numToVar(); ++v)
    setlibMeshSolutionValuesFromMFEM(v, getActiveFromProblem());
}

void
MultiAppMFEMTolibMeshShapeEvaluationTransfer::setlibMeshSolutionValuesFromMFEM(
    const unsigned int var_index, MFEMProblem & from_problem)
{
  /// The target variables
  std::vector<MooseVariableFieldBase *> _to_variables;
  if (_to_problems.size())
  {
    _to_variables.resize(_to_var_names.size());
    for (const auto i_var : index_range(_to_var_names))
      _to_variables[i_var] = &_to_problems[0]->getVariable(
          0, _to_var_names[i_var], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
  }

  // get libMesh and MFEM variables
  const auto & var_name = getToVarName(var_index);
  auto & from_var = from_problem.getProblemData().gridfunctions.GetRef(getFromVarName(var_index));
  for (const auto problem_id : index_range(_to_problems))
  {
    // auto & dofobject_to_val = dofobject_to_valsvec[problem_id];

    // libMesh EquationSystems
    // NOTE: we would expect to set variables from the displaced equation system here
    // auto & es = getEquationSystem(*_to_problems[problem_id], false);
    auto & es = _to_problems[problem_id]->es();
    // libMesh system
    System * to_sys = find_sys(es, var_name);

    // libMesh mesh
    const MeshBase & to_mesh = _to_problems[problem_id]->mesh(_displaced_target_mesh).getMesh();
    auto var_num = to_sys->variable_number(var_name);
    auto sys_num = to_sys->number();
    auto & fe_type = _to_variables[var_index]->feType();
    bool is_nodal = _to_variables[var_index]->isNodal();

    // Populate set of points
    std::vector<Point> outgoing_libmesh_points;
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
      for (const auto & elem :
           as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;
        outgoing_libmesh_points.push_back(elem->vertex_average());
      }
    }

    // Perform interpolation
    const mfem::Ordering::Type ordering = mfem::Ordering::byVDIM;
    mfem::Vector outgoing_mfem_points = Moose::MFEM::libMeshPointsToMFEMVector(
        outgoing_libmesh_points, to_mesh.mesh_dimension(), ordering);
    _mfem_interpolator.FindPoints(outgoing_mfem_points, ordering);
    mfem::Vector interp_vals;
    _mfem_interpolator.Interpolate(from_var, interp_vals);

    // Update libMesh solution DoFs with interpolated MFEM values
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
        to_sys->solution->set(dof, val);
        mfem_point_index++;
      }
    }
    else // Elemental, constant monomial
    {
      for (const auto & elem :
           as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Skip this element if the variable has no dofs at it.
        if (elem->n_dofs(sys_num, var_num) < 1)
          continue;
        const auto dof_object_id = elem->id();
        const DofObject * dof_object = to_mesh.elem_ptr(dof_object_id);
        const auto dof = dof_object->dof_number(sys_num, var_num, 0);
        const auto val = interp_vals[mfem_point_index];
        to_sys->solution->set(dof, val);
        mfem_point_index++;
      }
    }
    to_sys->solution->close();
    // Sync local solutions
    to_sys->update();
  }
}

#endif
