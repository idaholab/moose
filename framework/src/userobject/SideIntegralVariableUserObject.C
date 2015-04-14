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

#include "SideIntegralVariableUserObject.h"

template<>
InputParameters validParams<SideIntegralVariableUserObject>()
{
  InputParameters params = validParams<SideIntegralUserObject>();
  params.addRequiredParam<VariableName>("variable", "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariableUserObject::SideIntegralVariableUserObject(const InputParameters & parameters) :
    SideIntegralUserObject(parameters),
    MooseVariableInterface(parameters, false),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{
  addMooseVariableDependency(mooseVariable());
}

Real
SideIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}


// DEPRECATED CONSTRUCTOR
SideIntegralVariableUserObject::SideIntegralVariableUserObject(const std::string & deprecated_name, InputParameters parameters) :
    SideIntegralUserObject(deprecated_name, parameters),
    MooseVariableInterface(parameters, false),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _grad_u(_var.gradSln())
{
  addMooseVariableDependency(mooseVariable());
}
