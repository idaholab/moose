//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoolControlDataValuePostprocessor.h"
#include "THMProblem.h"

registerMooseObject("ThermalHydraulicsApp", BoolControlDataValuePostprocessor);

InputParameters
BoolControlDataValuePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += THMAppInterface::validParams();

  params.addRequiredParam<std::string>("control_data_name",
                                       "The name of the control data to output.");

  params.addClassDescription("Copies a boolean control data value.");

  return params;
}

BoolControlDataValuePostprocessor::BoolControlDataValuePostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    THMAppInterface(parameters),

    _control_data_value(getTHMApp().getTHMProblem().getControlData<bool>(
        getParam<std::string>("control_data_name")))
{
}

void
BoolControlDataValuePostprocessor::initialize()
{
}

void
BoolControlDataValuePostprocessor::execute()
{
}

Real
BoolControlDataValuePostprocessor::getValue()
{
  if (_control_data_value->get())
    return 1.;
  else
    return 0.;
}
