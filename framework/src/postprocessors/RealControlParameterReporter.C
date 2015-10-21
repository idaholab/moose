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

#include "RealControlParameterReporter.h"
#include "SubProblem.h"

template<>
InputParameters validParams<RealControlParameterReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params += validParams<ControlInterface>();

  params.addRequiredParam<std::string>("parameter", "The input parameter to control.");

  return params;
}

RealControlParameterReporter::RealControlParameterReporter(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    ControlInterface(parameters)
{
}

void
RealControlParameterReporter::initialSetup()
{
  _parameter = &getControllableValue<Real>("parameter");
}

Real
RealControlParameterReporter::getValue()
{
  return *_parameter;
}
