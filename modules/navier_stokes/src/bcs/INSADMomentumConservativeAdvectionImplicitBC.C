//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumConservativeAdvectionImplicitBC.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSADMomentumConservativeAdvectionImplicitBC);

InputParameters
INSADMomentumConservativeAdvectionImplicitBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Adds the advective term to the INS momentum equation");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density");
  return params;
}

INSADMomentumConservativeAdvectionImplicitBC::INSADMomentumConservativeAdvectionImplicitBC(
    const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters), _rho(getADMaterialProperty<Real>(NS::density))
{
}

ADReal
INSADMomentumConservativeAdvectionImplicitBC::computeQpResidual()
{
  return outer_product(_u[_qp], _u[_qp]) * _normals[_qp] * _test[_i][_qp] * _rho[_qp];
}
