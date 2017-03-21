/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHInterface.h"

template <>
InputParameters
validParams<CHInterface>()
{
  InputParameters params = CHInterfaceBase<Real>::validParams();
  params.addClassDescription(
      "Gradient energy Cahn-Hilliard Kernel with a scalar (isotropic) mobility");
  return params;
}

CHInterface::CHInterface(const InputParameters & parameters) : CHInterfaceBase<Real>(parameters) {}
