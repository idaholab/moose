//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
