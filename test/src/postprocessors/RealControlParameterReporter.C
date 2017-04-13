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

// MOOSE includes
#include "RealControlParameterReporter.h"
#include "SubProblem.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"

template <>
InputParameters
validParams<RealControlParameterReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<std::string>("parameter",
                                       "The input parameter to control, the name must "
                                       "be complete (e.g. "
                                       "Kernels/object/param_name).");
  return params;
}

RealControlParameterReporter::RealControlParameterReporter(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _parameter(NULL)
{
}

void
RealControlParameterReporter::initialSetup()
{
  MooseObjectParameterName name(getParam<std::string>("parameter"));
  const InputParameters & params =
      getMooseApp().getInputParameterWarehouse().getInputParametersObject(
          name.tag(), name.name(), _tid);
  if (!params.isParamValid(name.parameter()))
    mooseError("Unable to locate the parameter: ", name);
  else
    _parameter = &(params.get<Real>(name.parameter()));
}

Real
RealControlParameterReporter::getValue()
{
  return *_parameter;
}
