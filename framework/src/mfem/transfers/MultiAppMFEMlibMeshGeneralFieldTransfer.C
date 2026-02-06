//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MultiAppMFEMlibMeshGeneralFieldTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "MFEMProblem.h"
#include "MFEMMesh.h"

registerMooseObject("MooseApp", MultiAppMFEMlibMeshGeneralFieldTransfer);

namespace Moose::MFEM
{
size_t
MFEMIndex(const size_t i_dim,
          const size_t i_point,
          const size_t num_dims,
          const size_t num_points,
          const mfem::Ordering::Type ordering)
{
  if (ordering == mfem::Ordering::byNODES)
  {
    return i_dim * num_points + i_point;
  }
  else // ordering == mfem::Ordering::byVDIM
  {
    return i_point * num_dims + i_dim;
  }
}

mfem::Vector
pointsToMFEMVector(const std::vector<Point> & points,
                   const unsigned int num_dims,
                   const mfem::Ordering::Type ordering)
{
  const unsigned int num_points = points.size();
  mfem::Vector mfem_points(num_points * num_dims);
  for (unsigned int i_point = 0; i_point < num_points; i_point++)
  {
    for (unsigned int i_dim = 0; i_dim < num_dims; i_dim++)
    {
      const size_t idx = MFEMIndex(i_dim, i_point, num_dims, num_points, ordering);

      mfem_points(idx) = points[i_point](i_dim);
    }
  }

  return mfem_points;
}
}

InputParameters
MultiAppMFEMlibMeshGeneralFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "AuxVariable to store transferred value in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "Variable to transfer from");
  params.addClassDescription("Copies variable values from MFEM subapp to libMesh.");
  return params;
}

MultiAppMFEMlibMeshGeneralFieldTransfer::MultiAppMFEMlibMeshGeneralFieldTransfer(InputParameters const & params)
  : MultiAppTransfer(params),
    _mfem_interpolator(this->comm().get()),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  auto bad_problem = [this]()
  {
    mooseError(type(),
               " only works with MFEMProblem based applications. Check that all your inputs "
               "involved in this transfer are MFEMProblem based");
  };

  // if (hasToMultiApp())
  // {
  //   if (!dynamic_cast<MFEMProblem *>(&getToMultiApp()->problemBase()))
  //     bad_problem();
  //   for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
  //     if (getToMultiApp()->hasLocalApp(i) &&
  //         !dynamic_cast<MFEMProblem *>(&getToMultiApp()->appProblemBase(i)))
  //       bad_problem();
  // }
  // if (hasFromMultiApp())
  // {
  //   if (!dynamic_cast<MFEMProblem *>(&getFromMultiApp()->problemBase()))
  //     bad_problem();
  //   for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
  //     if (getFromMultiApp()->hasLocalApp(i) &&
  //         !dynamic_cast<MFEMProblem *>(&getFromMultiApp()->appProblemBase(i)))
  //       bad_problem();
  // }
}

void
MultiAppMFEMlibMeshGeneralFieldTransfer::transfer(FEProblemBase & to_problem, FEProblemBase & from_problem)
{
  auto * to_mfem_problem_ptr = dynamic_cast<MFEMProblem *>(&to_problem); 
  auto * from_mfem_problem_ptr = dynamic_cast<MFEMProblem *>(&from_problem);

  if (numToVar() != numFromVar())
    mooseError("Number of variables transferred must be same in both systems.");

  // TODO: tidy switch statement
  if (!to_mfem_problem_ptr && !from_mfem_problem_ptr)
    mooseError("No MFEM problem found in either source or destination app.");    

  if (to_mfem_problem_ptr && from_mfem_problem_ptr)
    mooseError("No libMesh problem found in either source or destination app.");

  // Send from MFEM problem to libMesh problem
  if (!to_mfem_problem_ptr && from_mfem_problem_ptr)
  {
    auto & mfem_mesh = from_mfem_problem_ptr->mesh().getMFEMParMesh();
    mfem_mesh.EnsureNodes();
    _mfem_interpolator.Setup(mfem_mesh);
    for (unsigned v = 0; v < numToVar(); ++v)
    {
      setlibMeshSolutionValuesFromMFEM(v, *from_mfem_problem_ptr);
      // Populate target points to pass to GSLib using MultiAppGeneralFieldTransfer::extractOutgoingPoints
      // interpolate using gslib
      // setSolutionVectorValues(i, dofobject_to_valsvec, interp_caches);
      // auto & to_var = to_problem.getProblemData().gridfunctions.GetRef(getToVarName(v));
      // auto & from_var = from_mfem_problem_ptr->getProblemData().gridfunctions.GetRef(getFromVarName(v));

      // to_var = from_var;
    }
  }
  // TODO: Send from libMesh problem to MFEM problem
  if (to_mfem_problem_ptr && !from_mfem_problem_ptr)
  {
  }
}

void
MultiAppMFEMlibMeshGeneralFieldTransfer::setlibMeshSolutionValuesFromMFEM(const unsigned int var_index, MFEMProblem & from_problem)
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
      mooseError("Transfers of non-nodal FEs of between libMesh and MFEM with order higher than CONSTANT are not supported.");
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
    mfem::Vector outgoing_mfem_points = Moose::MFEM::pointsToMFEMVector(outgoing_libmesh_points,
                  to_mesh.mesh_dimension(),
                  ordering);
    _mfem_interpolator.FindPoints(outgoing_mfem_points, ordering);
    mfem::Vector interp_vals;
    _mfem_interpolator.Interpolate(from_var, interp_vals);
    
    // Update libMesh solution DoFs with interpolated MFEM values
    unsigned int mfem_point_index = 0;
    if (fe_type.order > CONSTANT && !is_nodal)
    {
      mooseError("Transfers of non-nodal FEs of between libMesh and MFEM with order higher than CONSTANT are not supported.");
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

void
MultiAppMFEMlibMeshGeneralFieldTransfer::execute()
{
  TIME_SECTION("MultiAppMFEMlibMeshGeneralFieldTransfer::execute", 5, "Copies variables");
  if (_current_direction == TO_MULTIAPP)
  {
    for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    {
      if (getToMultiApp()->hasLocalApp(i))
      {
        transfer(getToMultiApp()->appProblemBase(i),
                 getToMultiApp()->problemBase());
      }
    }
  }
  else if (_current_direction == FROM_MULTIAPP)
  {
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        transfer(getFromMultiApp()->problemBase(),
                 getFromMultiApp()->appProblemBase(i));
      }
    }
  }
  else if (_current_direction == BETWEEN_MULTIAPP)
  {
    int transfers_done = 0;
    for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
    {
      if (getFromMultiApp()->hasLocalApp(i))
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          transfer(getToMultiApp()->appProblemBase(i),
                   getFromMultiApp()->appProblemBase(i));
          transfers_done++;
        }
      }
    }
    if (!transfers_done)
      mooseError("BETWEEN_MULTIAPP transfer not supported if there is not at least one subapp "
                 "per multiapp involved on each rank");
  }
}

void
MultiAppMFEMlibMeshGeneralFieldTransfer::checkSiblingsTransferSupported() const
{
  // Check that we are in the supported configuration: same number of source and target apps
  // The allocation of the child apps on the processors must be the same
  if (getFromMultiApp()->numGlobalApps() == getToMultiApp()->numGlobalApps())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) + getToMultiApp()->hasLocalApp(i) == 1)
        mooseError("Child application allocation on parallel processes must be the same to support "
                   "siblings variable field copy transfer");
  }
  else
    mooseError("Number of source and target child apps must match for siblings transfer");
}

#endif
