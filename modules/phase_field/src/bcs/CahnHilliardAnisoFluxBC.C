/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CahnHilliardAnisoFluxBC.h"

template <>
InputParameters
validParams<CahnHilliardAnisoFluxBC>()
{
  InputParameters params = CahnHilliardFluxBCBase<RealTensorValue>::validParams();
  params.addClassDescription(
      "Cahn-Hilliard fixed flux boundary condition using a tensorial (anisotropic) mobility");
  return params;
}

CahnHilliardAnisoFluxBC::CahnHilliardAnisoFluxBC(const InputParameters & parameters)
  : CahnHilliardFluxBCBase<RealTensorValue>(parameters)
{
}
