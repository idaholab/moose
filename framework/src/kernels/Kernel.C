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

#include "Kernel.h"
#include "Moose.h"
#include "MooseSystem.h"
#include "DofData.h"
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
InputParameters validParams<Kernel>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  return params;
}


Kernel::Kernel(const std::string & name, InputParameters parameters):
  PDEBase(name, parameters, *parameters.get<MooseSystem *>("_moose_system")->_element_data[parameters.get<THREAD_ID>("_tid")]),
  MaterialPropertyInterface(parameters.get<MooseSystem *>("_moose_system")->_material_data[_tid]),
   _dof_data(_moose_system._dof_data[_tid]),
   _element_data(*_moose_system._element_data[_tid]),
   _u(_element_data._var_vals[_var_num]),
   _grad_u(_element_data._var_grads[_var_num]),
   _second_u(_element_data._var_seconds[_var_num]),
   _u_dot(_element_data._var_dots[_var_num]),
   _du_dot_du(_element_data._var_du_dot_dus[_var_num]),
   _u_old(_element_data._var_vals_old[_var_num]),
   _u_older(_element_data._var_vals_older[_var_num]),
   _grad_u_old(_element_data._var_grads_old[_var_num]),
   _grad_u_older(_element_data._var_grads_older[_var_num]),
   _test((_element_data._test)[_var_num]),
   _grad_test(*(_element_data._grad_phi)[_fe_type]),
   _second_test(*(_element_data._second_phi)[_fe_type]),
   _u_old_newton(_element_data._var_vals_old_newton[_var_num]),
   _grad_u_old_newton(_element_data._var_grads_old_newton[_var_num])
{
  // If this variable isn't known yet... make it so
  _element_data._var_nums.insert(_var_num);
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(_moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getVariableNumber(coupled_var_name);
      _element_data._var_nums.insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (_moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = _moose_system.getAuxVariableNumber(coupled_var_name);
      _element_data._aux_var_nums.insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

void
Kernel::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","Kernel");
  
  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];

  precalculateResidual();
  for (_i=0; _i<_phi.size(); _i++)
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      var_Re(_i) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpResidual();  
//  Moose::perf_log.pop("computeResidual()","Kernel");
}

void
Kernel::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()",_name);

  DenseMatrix<Number> & var_Ke = *_dof_data._var_Kes[_var_num];


  for (_i=0; _i<_phi.size(); _i++)
    for (_j=0; _j<_phi.size(); _j++)
      for (_qp=0; _qp<_qrule->n_points(); _qp++)
        var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian();
  
//  Moose::perf_log.pop("computeJacobian()",_name);
}

void
Kernel::computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  for (_i=0; _i<_phi.size(); _i++)
    for (_j=0; _j<_phi.size(); _j++)
      for (_qp=0; _qp<_qrule->n_points(); _qp++)
      {
        if(jvar == _var_num)
          Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpJacobian();
        else
          Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*_JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }
  
//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}


Real
Kernel::computeIntegral()
{
//  Moose::perf_log.push("computeIntegral()",_name);

  Real sum = 0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  
//  Moose::perf_log.pop("computeIntegral()",_name);
  return sum;
}

Real
Kernel::computeQpJacobian()
{
  return 0;
}

Real
Kernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

Real
Kernel::computeQpIntegral()
{
  return 0;
}

void
Kernel::precalculateResidual()
{
}
