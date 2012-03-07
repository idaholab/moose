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

#include "PlotFunction.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PlotFunction>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("function", "Name of the function to plot (i.e. sample)");
  return params;
}

PlotFunction::PlotFunction(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _func(getFunction("function"))
{
}

PlotFunction::~PlotFunction()
{
}

void
PlotFunction::initialize()
{
}

void
PlotFunction::execute()
{
}

Real
PlotFunction::getValue()
{
  Point p;
  return _func.value(_t, p);
}
