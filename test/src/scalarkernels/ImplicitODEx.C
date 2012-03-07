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

template<>
InputParameters validParams<ImplicitODEx>()
{
  InputParameters params = validParams<ODEKernel>();
  params.addCoupledVar("y", "Y");
  return params;
}

ImplicitODEx::ImplicitODEx(const std::string & name, InputParameters parameters) :
    ODEKernel(name, parameters),
    _y_var(coupledScalar("y")),
    _y(coupledScalarValue("y"))
{
}

ImplicitODEx::~ImplicitODEx()
{
}

Real
ImplicitODEx::computeQpResidual()
{
  return -3. * _u[_i] - 2. * _y[_i];
}

Real
ImplicitODEx::computeQpJacobian()
{
  return -3.;
}

Real
ImplicitODEx::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _y_var)
    return -2.;
  else
    return 0.;
}
