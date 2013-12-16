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

#include "ScalarVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<ScalarVariable>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "Name of the variable");
  params.addParam<unsigned int>("component", 0, "Component to output for this variable");
  return params;
}

ScalarVariable::ScalarVariable(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _idx(getParam<unsigned int>("component"))
{
}

ScalarVariable::~ScalarVariable()
{
}

void
ScalarVariable::initialize()
{
}

void
ScalarVariable::execute()
{
}

Real
ScalarVariable::getValue()
{
  _var.reinit();
  return _var.sln()[_idx];
}
