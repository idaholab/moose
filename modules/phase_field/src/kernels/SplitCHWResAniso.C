/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHWResAniso.h"

template <>
InputParameters
validParams<SplitCHWResAniso>()
{
  InputParameters params = SplitCHWResBase<RealTensorValue>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a tensor (anisotropic) mobility");
  return params;
}

SplitCHWResAniso::SplitCHWResAniso(const InputParameters & parameters)
  : SplitCHWResBase<RealTensorValue>(parameters)
{
}
