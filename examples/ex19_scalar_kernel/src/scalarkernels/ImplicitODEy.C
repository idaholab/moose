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

#include "ImplicitODEy.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<ImplicitODEy>()
{
  InputParameters params = validParams<ODEKernel>();
  params.addCoupledVar("x", "variable X coupled into this kernel");
  return params;
}

ImplicitODEy::ImplicitODEy(const std::string & name, InputParameters parameters) :
    // You must call the constructor of the base class first
    ODEKernel(name, parameters),
    // get the coupled variable number and values
    _x_var(coupledScalar("x")),
    _x(coupledScalarValue("x"))
{
}

Real
ImplicitODEy::computeQpResidual()
{
  // the term of the ODE without the time derivative term
  return -4 * _x[_i] - _u[_i];
}

Real
ImplicitODEy::computeQpJacobian()
{
  // dF/dy
  return -1.;
}

Real
ImplicitODEy::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _x_var)
    return -4.;         // dF/dx
  else
    return 0.;          // everything else
}
