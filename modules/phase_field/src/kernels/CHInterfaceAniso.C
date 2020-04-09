//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHInterfaceAniso.h"

registerMooseObject("PhaseFieldApp", CHInterfaceAniso);

InputParameters
CHInterfaceAniso::validParams()
{
  InputParameters params = CHInterfaceBase<RealTensorValue>::validParams();
  params.addClassDescription(
      "Gradient energy Cahn-Hilliard Kernel with a tensor (anisotropic) mobility");
  return params;
}

CHInterfaceAniso::CHInterfaceAniso(const InputParameters & parameters)
  : CHInterfaceBase<RealTensorValue>(parameters)
{
}
