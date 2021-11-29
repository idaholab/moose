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

registerMooseObject("MooseTestApp", GeneralSetupInterfaceCount);
registerMooseObject("MooseTestApp", ElementSetupInterfaceCount);
registerMooseObject("MooseTestApp", SideSetupInterfaceCount);
registerMooseObject("MooseTestApp", InternalSideSetupInterfaceCount);
registerMooseObject("MooseTestApp", NodalSetupInterfaceCount);

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
