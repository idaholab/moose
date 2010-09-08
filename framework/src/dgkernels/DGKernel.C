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

const unsigned int DGKernel::InternalBndId = 123456789;

template<>
InputParameters validParams<DGKernel>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.set<bool>("_integrated") = true;
  params.addPrivateParam<unsigned int>("_boundary_id", DGKernel::InternalBndId);
  return params;
}


DGKernel::DGKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters):
  PDEBase(name, moose_system, parameters, *moose_system._face_data[parameters.get<THREAD_ID>("_tid")]),
  TwoMaterialPropertyInterface(moose_system._material_data[_tid], moose_system._neighbor_material_data[_tid]),
  _dof_data(moose_system._dof_data[_tid]),
  _face_data(*moose_system._face_data[_tid]),
  _neighbor_dof_data(moose_system._neighbor_dof_data[_tid]),
  _neighbor_face_data(*moose_system._neighbor_face_data[_tid]),
  _boundary_id(parameters.get<unsigned int>("_boundary_id")),
  _side_elem(NULL),
  _neighbor_elem(NULL),
  _current_side(_face_data._current_side),
  _current_side_elem(_face_data._current_side_elem),
  _u(_face_data._var_vals[_var_num]),
  _grad_u(_face_data._var_grads[_var_num]),
  _second_u(_face_data._var_seconds[_var_num]),
  // TODO: Fix this holy hack!
  _test(*_face_data._phi[_fe_type]),
  _grad_test(*_face_data._grad_phi[_fe_type]),
  _second_test(*_face_data._second_phi[_fe_type]),
  _normals(*_face_data._normals[_fe_type]),
  _phi_neighbor(*_neighbor_face_data._phi[_fe_type]),
  _grad_phi_neighbor(*_neighbor_face_data._grad_phi[_fe_type]),
  _second_phi_neighbor(*_neighbor_face_data._second_phi[_fe_type]),
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
  _face_data._var_nums.insert(_var_num);
  _neighbor_face_data._var_nums.insert(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getVariableNumber(coupled_var_name);
      _face_data._var_nums.insert(coupled_var_num);
      _neighbor_face_data._var_nums.insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getAuxVariableNumber(coupled_var_name);
      _face_data._aux_var_nums.insert(coupled_var_num);
      _neighbor_face_data._aux_var_nums.insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

DGKernel::~DGKernel()
{
}

void
DGKernel::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","DGKernel");

  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];
  DenseSubVector<Number> & neighbor_var_Re = *_neighbor_dof_data._var_Res[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_phi.size(); _i++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpResidual(Element);

    for (_i=0; _i<_phi_neighbor.size(); _i++)
      neighbor_var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpResidual(Neighbor);
  }

//  Moose::perf_log.pop("computeResidual()","DGKernel");
}

void
DGKernel::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","DGKernel");

  DenseMatrix<Number> & var_Kee = *_dof_data._var_Kes[_var_num];
  DenseMatrix<Number> & var_Ken = *_dof_data._var_Kns[_var_num];

  DenseMatrix<Number> & var_Kne = *_neighbor_dof_data._var_Kns[_var_num];
  DenseMatrix<Number> & var_Knn = *_neighbor_dof_data._var_Kes[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Kee(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian(ElementElement);

    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Ken(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian(ElementNeighbor);

    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Kne(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian(NeighborElement);

    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        var_Knn(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian(NeighborNeighbor);
  }

//  Moose::perf_log.pop("computeJacobian()","DGKernel");
}


VariableValue &
DGKernel::coupledNeighborValue(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _neighbor_face_data._var_vals[_coupled_vars[varname][i]._num];
  else
    return _neighbor_face_data._aux_var_vals[_coupled_aux_vars[varname][i]._num];
}

VariableGradient &
DGKernel::coupledNeighborGradient(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _neighbor_face_data._var_grads[_coupled_vars[varname][i]._num];
  else
    return _neighbor_face_data._aux_var_grads[_coupled_aux_vars[varname][i]._num];
}

VariableSecond &
DGKernel::coupledNeighborSecond(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  //Aux vars can't have second derivatives!
  return _neighbor_face_data._var_seconds[_coupled_vars[varname][i]._num];
}

VariableValue &
DGKernel::coupledNeighborValueOld(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _neighbor_face_data._var_vals_old[_coupled_vars[varname][i]._num];
  else
    return _neighbor_face_data._aux_var_vals_old[_coupled_aux_vars[varname][i]._num];
}

VariableValue &
DGKernel::coupledNeighborValueOlder(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  if(!isAux(varname))
    return _neighbor_face_data._var_vals_older[_coupled_vars[varname][i]._num];
  else
    return _neighbor_face_data._aux_var_vals_older[_coupled_aux_vars[varname][i]._num];
}

VariableGradient &
DGKernel::coupledNeighborGradientOld(std::string varname, int i)
{
  if(!isCoupled(varname))
    mooseError("\nObject " + name() + " was not provided with a coupled variable " + varname + "\n\n");

  return _neighbor_face_data._var_grads_old[_coupled_vars[varname][i]._num];
}
