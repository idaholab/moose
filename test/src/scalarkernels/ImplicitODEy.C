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

template<>
InputParameters validParams<ImplicitODEy>()
{
  InputParameters params = validParams<ODEKernel>();
  params.addCoupledVar("x", "X");
  return params;
}

ImplicitODEy::ImplicitODEy(const std::string & name, InputParameters parameters) :
    ODEKernel(name, parameters),
    _x_var(coupledScalar("x")),
    _x(coupledScalarValue("x"))
{
}

ImplicitODEy::~ImplicitODEy()
{
}

Real
ImplicitODEy::computeQpResidual()
{
  return -4 * _x[_i] - _u[_i];
}

Real
ImplicitODEy::computeQpJacobian()
{
  return -1.;
}

Real
ImplicitODEy::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _x_var)
    return -4.;
  else
    return 0.;
}
