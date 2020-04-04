//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CahnHilliard.h"

registerMooseObject("PhaseFieldApp", CahnHilliard);

InputParameters
CahnHilliard::validParams()
{
  InputParameters params = CahnHilliardBase<Real>::validParams();
  params.addClassDescription("Cahn-Hilliard Kernel that uses a DerivativeMaterial Free Energy and "
                             "a scalar (isotropic) mobility");
  return params;
}

CahnHilliard::CahnHilliard(const InputParameters & parameters) : CahnHilliardBase<Real>(parameters)
{
}
