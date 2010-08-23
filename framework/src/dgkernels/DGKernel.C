#include "DGKernel.h"
#include "Moose.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "MaterialData.h"
#include "MaterialFactory.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "libmesh_common.h"

template<>
InputParameters validParams<DGKernel>()
{
  // FIXME: make it a descendant of BoundaryCondition
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.set<bool>("_integrated") = true;
  return params;
}


DGKernel::DGKernel(std::string name, MooseSystem & moose_system, InputParameters parameters):
  BoundaryCondition(name, moose_system, parameters),
  _neighbor_elem(NULL),
  _neighbor_dof_data(moose_system._neighbor_dof_data[_tid]),
  _neighbor_face_data(*moose_system._neighbor_face_data[_tid]),
  _test_neighbor(*_neighbor_face_data._phi[_fe_type]),
  _grad_test_neighbor(*_neighbor_face_data._grad_phi[_fe_type]),
  _second_test_neighbor(*_neighbor_face_data._second_phi[_fe_type]),
  _u_neighbor(_neighbor_face_data._var_vals[_var_num]),
  _grad_u_neighbor(_neighbor_face_data._var_grads[_var_num]),
  _second_u_neighbor(_neighbor_face_data._var_seconds[_var_num]),
  _u_old_neighbor(_neighbor_face_data._var_vals_old[_var_num]),
  _u_older_neighbor(_neighbor_face_data._var_vals_older[_var_num]),
  _grad_u_old_neighbor(_neighbor_face_data._var_grads_old[_var_num]),
  _grad_u_older_neighbor(_neighbor_face_data._var_grads_older[_var_num])
{
  // If this variable isn't known yet... make it so
  _neighbor_face_data._var_nums[_boundary_id].insert(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getVariableNumber(coupled_var_name);
      _neighbor_face_data._var_nums[_boundary_id].insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getAuxVariableNumber(coupled_var_name);
      _neighbor_face_data._aux_var_nums[_boundary_id].insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

DGKernel::~DGKernel()
{
}
