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

#include "PrintScalarVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintScalarVariable>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "Name of the variable");
  params.addParam<unsigned int>("idx", 0, "Index for this variable");
  return params;
}

PrintScalarVariable::PrintScalarVariable(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _idx(getParam<unsigned int>("idx"))
{
}

PrintScalarVariable::~PrintScalarVariable()
{
}

void
PrintScalarVariable::initialize()
{
}

void
PrintScalarVariable::execute()
{
}

Real
PrintScalarVariable::getValue()
{
  _var.reinit();
  return _var.sln()[_idx];
}
