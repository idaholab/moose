/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CahnHilliardAniso.h"

template <>
InputParameters
validParams<CahnHilliardAniso>()
{
  InputParameters params = CahnHilliardBase<RealTensorValue>::validParams();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy and "
                             "a tensor (anisotropic) mobility");
  return params;
}

CahnHilliardAniso::CahnHilliardAniso(const InputParameters & parameters)
  : CahnHilliardBase<RealTensorValue>(parameters)
{
}
