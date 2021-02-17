//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorosityMomentumPressure.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVPorosityMomentumPressure);

InputParameters
FVPorosityMomentumPressure::validParams()
{
  InputParameters params = FVMomentumPressure::validParams();
  params.addClassDescription("Introduces the coupled pressure term multiplied by a porosity into "
                             "the Navier-Stokes momentum equation.");
  params.addParam<MaterialPropertyName>(NS::porosity, NS::porosity, "The porosity");
  return params;
}

FVPorosityMomentumPressure::FVPorosityMomentumPressure(const InputParameters & params)
  : FVMomentumPressure(params), _eps(getMaterialProperty<Real>(NS::porosity))
{
}

ADReal
FVPorosityMomentumPressure::computeQpResidual()
{
  return _eps[_qp] * FVMomentumPressure::computeQpResidual();
}
