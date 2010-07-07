//MOOSE includes
#include "Moose.h"
#include "MooseSystem.h"
#include "ElementData.h"
#include "BoundaryCondition.h"

//libMesh includes
#include "numeric_vector.h"
#include "dof_map.h"

template<>
InputParameters validParams<BoundaryCondition>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this boundary condition applies");
  return params;
}

BoundaryCondition::BoundaryCondition(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  PDEBase(name, moose_system, parameters, moose_system._face_data),
  MaterialPropertyInterface(moose_system._material_data),
   _element_data(moose_system._element_data),
   _face_data(moose_system._face_data),
   _boundary_id(parameters.get<unsigned int>("_boundary_id")),
   _side_elem(NULL),
   _material(_moose_system._face_data._material[_tid]),
   _normals(*moose_system._face_data._normals[_tid][_fe_type]),
   _current_side(moose_system._face_data._current_side[_tid]),
   _current_node(moose_system._face_data._current_node[_tid]),
   _current_residual(moose_system._face_data._current_residual[_tid]),
   _u(_integrated ? moose_system._face_data._var_vals[_tid][_var_num] : moose_system._face_data._var_vals_nodal[_tid][_var_num]),
   _grad_u(_integrated ? moose_system._face_data._var_grads[_tid][_var_num] : moose_system._grad_zero[_tid]),
   _second_u(_integrated ? moose_system._face_data._var_seconds[_tid][_var_num] : moose_system._second_zero[_tid]),
   // TODO: Fix this holy hack!
   _test(*moose_system._face_data._phi[_tid][_fe_type]), 
   _grad_test(*moose_system._face_data._grad_phi[_tid][_fe_type]), 
   _second_test(*moose_system._face_data._second_phi[_tid][_fe_type])
{
  // If this variable isn't known yet... make it so
  if(_integrated)
    _face_data._var_nums[_boundary_id].insert(_var_num);
  else
    _face_data._boundary_to_var_nums_nodal[_boundary_id].insert(_var_num);

  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getVariableNumber(coupled_var_name);
      if(_integrated)
        _face_data._var_nums[_boundary_id].insert(coupled_var_num);
      else
        _face_data._boundary_to_var_nums_nodal[_boundary_id].insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getAuxVariableNumber(coupled_var_name);
      _face_data._aux_var_nums[0].insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

unsigned int
BoundaryCondition::boundaryID()
{
  return _boundary_id;
}

void
BoundaryCondition::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","BoundaryCondition");

  DenseSubVector<Number> & var_Re = *_element_data._var_Res[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      for (_i=0; _i<_phi.size(); _i++)
        var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpResidual();
  else
  {
    //Use _qp to keep things standard at the leaf level
    //_qp is really looping over nodes right now.
    for(_qp=0; _qp<_current_elem->n_nodes(); _qp++)
      if(_current_elem->is_node_on_side(_qp,_current_side))
        var_Re(_qp) = _moose_system._scaling_factor[_var_num]*computeQpResidual();
  }

//  Moose::perf_log.pop("computeResidual()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","BoundaryCondition");

  DenseMatrix<Number> & var_Ke = *_element_data._var_Kes[_tid][_var_num];

  if(_integrated)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      for (_i=0; _i<_phi.size(); _i++)
        for (_j=0; _j<_phi.size(); _j++)
          var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian();
  else
  {
    for(_i=0; _i<_phi.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi.size(); _j++)
          var_Ke(_i,_j) = 0;
        
        var_Ke(_i,_i) = _moose_system._scaling_factor[_var_num]*1;
      }
    }
  }

//  Moose::perf_log.pop("computeJacobian()","BoundaryCondition");
}

void
BoundaryCondition::computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar)
{
//  Moose::perf_log.push("computeJacobianBlock()","BoundaryCondition");

  if(_integrated)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      for (_i=0; _i<_phi.size(); _i++)
        for (_j=0; _j<_phi.size(); _j++)
        {
          if(ivar ==jvar)
            Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
          else
            Ke(_i,_j) += _JxW[_qp]*computeQpOffDiagJacobian(jvar);
        }
  
  else
  {
    for(_i=0; _i<_phi.size(); _i++)
    {
      if(_current_elem->is_node_on_side(_i,_current_side))
      {
        //Zero out the row and put 1 on the diagonal
        for(_j=0; _j<_phi.size(); _j++)
          Ke(_i,_j) = 0;

        if(ivar ==jvar)
          Ke(_i,_i) = 1;
      }
    }
  }

//  Moose::perf_log.pop("computeJacobianBlock()","BoundaryCondition");
}

void
BoundaryCondition::computeAndStoreResidual()
{
  _qp = 0;
  _current_residual->set(_moose_system._face_data._nodal_bc_var_dofs[_tid][_var_num], _moose_system._scaling_factor[_var_num]*computeQpResidual());
}

Real
BoundaryCondition::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  
//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}

MooseArray<Real> &
BoundaryCondition::coupledValue(std::string var_name, int i)
{
  if(!isCoupled(var_name, i))
    mooseError("BC _" + name() + "_ was not provided with a variable coupled_as " + var_name + "\n\n");
  
  if(_integrated)
    return PDEBase::coupledValue(var_name, i);

  return _moose_system._face_data._var_vals_nodal[_tid][_coupled_vars[var_name][i]._num];
}

MooseArray<RealGradient> &
BoundaryCondition::coupledGradient(std::string var_name, int i)
{
  if(!isCoupled(var_name, i))
    mooseError("BC _" + name() + "_ was not provided with a variable coupled_as " + var_name + "\n\n");

  if(_integrated)
    return PDEBase::coupledGradient(var_name, i);

  mooseError("Integrated BC required for coupled Gradient type\n");
}

Real
BoundaryCondition::computeQpJacobian()
{
  return 0;
}

Real
BoundaryCondition::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

Real
BoundaryCondition::computeQpIntegral()
{
  return 0;
}
