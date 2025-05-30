//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMInterfaceKernelSmallStrain.h"

registerMooseObject("SolidMechanicsApp", ADCZMInterfaceKernelSmallStrain);

InputParameters
ADCZMInterfaceKernelSmallStrain::validParams()
{
  InputParameters params = ADCZMInterfaceKernelBase::validParams();
  params.addClassDescription(
      "CZM Interface kernel to use when using the small strain kinematic formulation.");
  return params;
}

ADCZMInterfaceKernelSmallStrain::ADCZMInterfaceKernelSmallStrain(const InputParameters & parameters)
  : ADCZMInterfaceKernelBase(parameters)
{
}
