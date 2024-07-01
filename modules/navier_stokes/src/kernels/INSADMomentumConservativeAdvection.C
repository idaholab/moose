//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumConservativeAdvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSADMomentumConservativeAdvection);

InputParameters
INSADMomentumConservativeAdvection::validParams()
{
  InputParameters params = ADVectorKernelGrad::validParams();
  params.addClassDescription("Adds the advective term to the INS momentum equation");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density");
  return params;
}

INSADMomentumConservativeAdvection::INSADMomentumConservativeAdvection(
    const InputParameters & parameters)
  : ADVectorKernelGrad(parameters), _rho(getADMaterialProperty<Real>(NS::density))
{
}

ADRealTensorValue
INSADMomentumConservativeAdvection::precomputeQpResidual()
{
  return -outer_product(_rho[_qp] * _u[_qp], _u[_qp]);
}
