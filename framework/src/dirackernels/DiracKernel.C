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

#include "DiracKernel.h"
#include "MooseSystem.h"

// libMesh includes
#include "parallel.h"
#include "point_locator_base.h"
#include "libmesh_common.h"

template<>
InputParameters validParams<DiracKernel>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  return params;
}

DiracKernel::DiracKernel(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  : PDEBase(name, moose_system, parameters, *moose_system._dirac_kernel_data[parameters.get<THREAD_ID>("_tid")]),
    _dirac_kernel_data(*_moose_system._dirac_kernel_data[_tid]),
    _dirac_kernel_info(_moose_system._dirac_kernel_info),
    _var_name(getParam<std::string>("variable")),
    _dof_data(moose_system._dof_data[_tid]),
    _u(_dirac_kernel_data._var_vals[_var_num]),
    _grad_u(_dirac_kernel_data._var_grads[_var_num]),
    _second_u(_dirac_kernel_data._var_seconds[_var_num]),
    _u_dot(_dirac_kernel_data._var_dots[_var_num]),
    _du_dot_du(_dirac_kernel_data._var_du_dot_dus[_var_num]),
    _u_old(_dirac_kernel_data._var_vals_old[_var_num]),
    _u_older(_dirac_kernel_data._var_vals_older[_var_num]),
    _grad_u_old(_dirac_kernel_data._var_grads_old[_var_num]),
    _grad_u_older(_dirac_kernel_data._var_grads_older[_var_num]),
    _test((_dirac_kernel_data._test)[_var_num]),
    _grad_test(*(_dirac_kernel_data._grad_phi)[_fe_type]),
    _second_test(*(_dirac_kernel_data._second_phi)[_fe_type]),
    _current_points(_dirac_kernel_data._current_points)
{
  // If this variable isn't known yet... make it so
  _dirac_kernel_data._var_nums.insert(_var_num);
  for(unsigned int i=0;i<_coupled_to.size();i++)
  {
    std::string coupled_var_name=_coupled_to[i];

    //Is it in the nonlinear system or the aux system?
    if(moose_system.hasVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getVariableNumber(coupled_var_name);
      _dirac_kernel_data._var_nums.insert(coupled_var_num);
    }
    //Look for it in the Aux system
    else if (moose_system.hasAuxVariable(coupled_var_name))
    {
      unsigned int coupled_var_num = moose_system.getAuxVariableNumber(coupled_var_name);
      _dirac_kernel_data._aux_var_nums.insert(coupled_var_num);
    }
    else
      mooseError("Coupled variable '" + coupled_var_name + "' not found.");
  }
}

void
DiracKernel::computeResidual()
{
  DenseSubVector<Number> & var_Re = *_dof_data._var_Res[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _current_point=_current_points[_qp];
    if(isActiveAtPoint(_current_elem, _current_point))
      for (_i=0; _i<_phi.size(); _i++)
        var_Re(_i) += _moose_system._scaling_factor[_var_num]*computeQpResidual();
  }
}

void
DiracKernel::computeJacobian()
{
  DenseMatrix<Number> & var_Ke = *_dof_data._var_Kes[_var_num];

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    _current_point=_current_points[_qp];    
    if(isActiveAtPoint(_current_elem, _current_point))
      for (_i=0; _i<_phi.size(); _i++)
        for (_j=0; _j<_phi.size(); _j++)
          var_Ke(_i,_j) += _moose_system._scaling_factor[_var_num]*computeQpJacobian();
  }
}

Real
DiracKernel::computeQpJacobian()
{
  return 0;
}

void
DiracKernel::addPoint(const Elem * elem, Point p)
{
  if(elem->processor_id() != libMesh::processor_id())
    return;

  _dirac_kernel_info.addPoint(elem, p);
  _elements.insert(elem);
  _points[elem].insert(p);
}

void
DiracKernel::addPoint(Point p)
{
  AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, *_moose_system.getMesh());
  const Elem * elem = (*pl)(p);
  addPoint(elem, p);
}

bool
DiracKernel::hasPointsOnElem(const Elem * elem)
{
  return _elements.count(elem) != 0;
}

bool
DiracKernel::isActiveAtPoint(const Elem * elem, const Point & p)
{
  return _points[elem].count(p) != 0;
}

