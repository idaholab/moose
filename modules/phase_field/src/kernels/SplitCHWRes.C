/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SplitCHWRes.h"

template <>
InputParameters
validParams<SplitCHWRes>()
{
  InputParameters params = SplitCHWResBase<Real>::validParams();
  params.addClassDescription("Split formulation Cahn-Hilliard Kernel for the chemical potential "
                             "variable with a scalar (isotropic) mobility");
  return params;
}

SplitCHWRes::SplitCHWRes(const InputParameters & parameters) : SplitCHWResBase<Real>(parameters) {}
