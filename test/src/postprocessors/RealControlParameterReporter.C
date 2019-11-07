//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RealControlParameterReporter.h"
#include "SubProblem.h"
#include "MooseObjectParameterName.h"
#include "InputParameterWarehouse.h"

registerMooseObject("MooseTestApp", RealControlParameterReporter);

InputParameters
RealControlParameterReporter::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
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
