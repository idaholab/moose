//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMInterfaceKernelTotalLagrangian.h"

registerMooseObject("SolidMechanicsApp", ADCZMInterfaceKernelTotalLagrangian);

InputParameters
ADCZMInterfaceKernelTotalLagrangian::validParams()
{
  InputParameters params = ADCZMInterfaceKernelBase::validParams();
  params.addClassDescription(
      "CZM Interface kernel to use when using the total Lagrangian formulation.");
  params.set<std::string>("traction_global_name") = "PK1traction";
  return params;
}

ADCZMInterfaceKernelTotalLagrangian::ADCZMInterfaceKernelTotalLagrangian(
    const InputParameters & parameters)
  : ADCZMInterfaceKernelBase(parameters)
{
}
