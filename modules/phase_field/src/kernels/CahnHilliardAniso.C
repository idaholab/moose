//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CahnHilliardAniso.h"

registerMooseObject("PhaseFieldApp", CahnHilliardAniso);

InputParameters
CahnHilliardAniso::validParams()
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
