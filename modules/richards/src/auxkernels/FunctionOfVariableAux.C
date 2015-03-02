/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "FunctionOfVariableAux.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionOfVariableAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("t_variable", "Variable that will be substituted into the 't' slot of the function");
  params.addRequiredParam<FunctionName>("function", "The function to use.  The t_variable gets substituted in the 't' slot of this function");
  params.addClassDescription("AuxVariable = function(t_variable, current_point).  That is, define a function of t and this will substitute your variable for t and give you the result as an AuxVariable");
  return params;
}

FunctionOfVariableAux::FunctionOfVariableAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _t_variable(coupledValue("t_variable")),
    _func(getFunction("function"))
{}

Real
FunctionOfVariableAux::computeValue()
{
  Real t_val = _t_variable[_qp];
  if (isNodal())
    return _func.value(t_val, *_current_node);
  else
    return _func.value(t_val, _current_elem->centroid());
}
