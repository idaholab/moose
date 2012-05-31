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

#include "ImplicitODEx.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<ImplicitODEx>()
{
  InputParameters params = validParams<ODEKernel>();
  params.addCoupledVar("y", "variable Y coupled into this kernel");
  return params;
}

ImplicitODEx::ImplicitODEx(const std::string & name, InputParameters parameters) :
    // You must call the constructor of the base class first
    ODEKernel(name, parameters),
    // get the coupled variable number and values
    _y_var(coupledScalar("y")),
    _y(coupledScalarValue("y"))
{
}

Real
ImplicitODEx::computeQpResidual()
{
  // the term of the ODE without the time derivative term
  return -3. * _u[_i] - 2. * _y[_i];
}

Real
ImplicitODEx::computeQpJacobian()
{
  // dF/dx
  return -3.;
}

Real
ImplicitODEx::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _y_var)
    return -2.;         // dF/dy
  else
    return 0.;          // everything else
}
