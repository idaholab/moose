//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CHInterface.h"

registerMooseObject("PhaseFieldApp", CHInterface);

InputParameters
CHInterface::validParams()
{
  InputParameters params = CHInterfaceBase<Real>::validParams();
  params.addClassDescription(
      "Gradient energy Cahn-Hilliard Kernel with a scalar (isotropic) mobility");
  return params;
}

CHInterface::CHInterface(const InputParameters & parameters) : CHInterfaceBase<Real>(parameters) {}
