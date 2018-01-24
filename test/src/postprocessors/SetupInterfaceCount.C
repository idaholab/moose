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
