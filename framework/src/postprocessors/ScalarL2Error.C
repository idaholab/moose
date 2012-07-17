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

#include "ScalarL2Error.h"
#include "Function.h"
#include "SubProblem.h"

template<>
InputParameters validParams<ScalarL2Error>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("variable", "The name of the scalar variable");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ScalarL2Error::ScalarL2Error(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<std::string>("variable"))),
    _func(getFunction("function"))
{
}

void
ScalarL2Error::initialize()
{
}

void
ScalarL2Error::execute()
{
}

Real
ScalarL2Error::getValue()
{
  _var.reinit();
  Point p;
  Real diff = (_var.sln()[0] - _func.value(_t, p));
  return std::sqrt(diff*diff);
}
