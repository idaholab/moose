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

#include "CoupledAux.h"

template<>
InputParameters validParams<CoupledAux>()
{
  InputParameters params = validParams<AuxKernel>();

  MooseEnum operators("+, -, *, /", "+");

  params.addRequiredCoupledVar("coupled", "Coupled Value for Calculation");

  params.addParam<Real>("value", 0.0, "A value to use in the binary arithmetic operation of this coupled auxkernel");
  params.addParam<MooseEnum>("operator", operators, "The binary operator to use in the calculation");
  return params;
}

CoupledAux::CoupledAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _value(getParam<Real>("value")),
    _operator(getParam<MooseEnum>("operator")),
    _coupled(coupled("coupled")),
    _coupled_val(coupledValue("coupled"))
{
}

Real
CoupledAux::computeValue()
{
  if (_operator == "+")
    return _coupled_val[_qp]+_value;
  else if (_operator == "-")
    return _coupled_val[_qp]-_value;
  else if (_operator == "*")
    return _coupled_val[_qp]*_value;
  else if (_operator == "/")
    // We are going to do division for this operation
    // This is useful for testing evalutation order
    // when we attempt to divide by zero!
    return _value / _coupled_val[_qp];

  // Won't reach this statement
  return 0;
}
