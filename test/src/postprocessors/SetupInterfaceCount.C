//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SetupInterfaceCount.h"

InputParameters
setupInterfaceCountParameters()
{
  InputParameters parameters = emptyInputParameters();
  MooseEnum count_type(
      "initial timestep subdomain linear nonlinear initialize finalize execute threadjoin");
  parameters.addRequiredParam<MooseEnum>(
      "count_type", count_type, "Specify the count type to return.");
  return parameters;
}

template <>
InputParameters
validParams<GeneralSetupInterfaceCount>()
{
  InputParameters parameters = validParams<GeneralPostprocessor>();
  parameters += setupInterfaceCountParameters();
  return parameters;
}

template <>
InputParameters
validParams<ElementSetupInterfaceCount>()
{
  InputParameters parameters = validParams<ElementPostprocessor>();
  parameters += setupInterfaceCountParameters();
  return parameters;
}

template <>
InputParameters
validParams<SideSetupInterfaceCount>()
{
  InputParameters parameters = validParams<SidePostprocessor>();
  parameters += setupInterfaceCountParameters();
  return parameters;
}

template <>
InputParameters
validParams<InternalSideSetupInterfaceCount>()
{
  InputParameters parameters = validParams<InternalSidePostprocessor>();
  parameters += setupInterfaceCountParameters();
  return parameters;
}

template <>
InputParameters
validParams<NodalSetupInterfaceCount>()
{
  InputParameters parameters = validParams<NodalPostprocessor>();
  parameters += setupInterfaceCountParameters();
  return parameters;
}

GeneralSetupInterfaceCount::GeneralSetupInterfaceCount(const InputParameters & parameters)
  : SetupInterfaceCount<GeneralPostprocessor>(parameters)
{
}

ElementSetupInterfaceCount::ElementSetupInterfaceCount(const InputParameters & parameters)
  : SetupInterfaceCount<ElementPostprocessor>(parameters)
{
}

SideSetupInterfaceCount::SideSetupInterfaceCount(const InputParameters & parameters)
  : SetupInterfaceCount<SidePostprocessor>(parameters)
{
}

InternalSideSetupInterfaceCount::InternalSideSetupInterfaceCount(const InputParameters & parameters)
  : SetupInterfaceCount<InternalSidePostprocessor>(parameters)
{
}

NodalSetupInterfaceCount::NodalSetupInterfaceCount(const InputParameters & parameters)
  : SetupInterfaceCount<NodalPostprocessor>(parameters)
{
}
