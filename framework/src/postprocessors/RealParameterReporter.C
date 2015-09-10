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

#include "RealParameterReporter.h"
#include "SubProblem.h"

template<>
InputParameters validParams<RealParameterReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params += validParams<ControlInterface>();

  params.addRequiredParam<std::string>("parameter", "The input parameter to control.");

  return params;
}

RealParameterReporter::RealParameterReporter(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    ControlInterface(parameters)
{
}

void
RealParameterReporter::initialSetup()
{
  _parameter = &getControlParam<Real>("parameter");
}

Real
RealParameterReporter::getValue()
{
  return *_parameter;
}
