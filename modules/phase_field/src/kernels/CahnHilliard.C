/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CahnHilliard.h"

template <>
InputParameters
validParams<CahnHilliard>()
{
  InputParameters params = CahnHilliardBase<Real>::validParams();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy and "
                             "a scalar (isotropic) mobility");
  return params;
}

CahnHilliard::CahnHilliard(const InputParameters & parameters) : CahnHilliardBase<Real>(parameters)
{
}
