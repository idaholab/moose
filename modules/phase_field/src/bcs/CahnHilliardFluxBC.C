/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CahnHilliardFluxBC.h"

template <>
InputParameters
validParams<CahnHilliardFluxBC>()
{
  InputParameters params = CahnHilliardFluxBCBase<Real>::validParams();
  params.addClassDescription(
      "Cahn-Hilliard fixed flux boundary condition using a scalar (isotropic) mobility");
  return params;
}

CahnHilliardFluxBC::CahnHilliardFluxBC(const InputParameters & parameters)
  : CahnHilliardFluxBCBase<Real>(parameters)
{
}
