/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHInterfaceAniso.h"

template <>
InputParameters
validParams<CHInterfaceAniso>()
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
