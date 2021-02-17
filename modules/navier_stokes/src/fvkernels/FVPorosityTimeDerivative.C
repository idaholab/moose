//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorosityTimeDerivative.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVPorosityTimeDerivative);

InputParameters
FVPorosityTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addClassDescription("A time derivative multiplied by a porosity material property");
  params.addParam<MaterialPropertyName>(NS::porosity, NS::porosity, "The porosity");
  return params;
}

FVPorosityTimeDerivative::FVPorosityTimeDerivative(const InputParameters & parameters)
  : FVTimeKernel(parameters), _eps(getMaterialProperty<Real>(NS::porosity))
{
}

ADReal
FVPorosityTimeDerivative::computeQpResidual()
{
  return _eps[_qp] * FVTimeKernel::computeQpResidual();
}
