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

#include "Damper.h"

// Moose includes
#include "MooseSystem.h"

template<>
InputParameters validParams<Damper>()
{
  InputParameters params = validParams<PDEBase>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this damper operates on");
  return params;
}

Damper::Damper(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :PDEBase(name, moose_system, parameters, *moose_system._element_data[parameters.get<THREAD_ID>("_tid")]),
   MaterialPropertyInterface(moose_system._material_data[_tid]),
   _damper_data(*moose_system._damper_data[_tid]),
   _element_data(*moose_system._element_data[_tid]),
   _u_increment(_damper_data._var_increments[_var_num]),
   _u(_element_data._var_vals[_var_num]),
   _grad_u(_element_data._var_grads[_var_num]),
   _second_u(_element_data._var_seconds[_var_num]),
   _u_old(_element_data._var_vals_old[_var_num]),
   _u_older(_element_data._var_vals_older[_var_num]),
   _grad_u_old(_element_data._var_grads_old[_var_num]),
   _grad_u_older(_element_data._var_grads_older[_var_num])
{}

Real
Damper::computeDamping()
{
  Real damping = 1.0;
  Real cur_damping = 1.0;
  
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    cur_damping = computeQpDamping();
    if(cur_damping < damping)
      damping = cur_damping;
  }

  return damping;
}

