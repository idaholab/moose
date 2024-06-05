//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMomentumConservativeAdvectionWeakDiriBC.h"
#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSADMomentumConservativeAdvectionWeakDiriBC);

InputParameters
INSADMomentumConservativeAdvectionWeakDiriBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addClassDescription("Adds the advective term to the INS momentum equation");
  params.addRequiredParam<MaterialPropertyName>(NS::density, "The density");
  params.addRequiredParam<FunctionName>("velocity", "The prescribed velocity");
  return params;
}

INSADMomentumConservativeAdvectionWeakDiriBC::INSADMomentumConservativeAdvectionWeakDiriBC(
    const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _rho(getADMaterialProperty<Real>(NS::density)),
    _diri_vel(getFunction("velocity"))
{
}

ADReal
INSADMomentumConservativeAdvectionWeakDiriBC::computeQpResidual()
{
  const auto diri_vel = _diri_vel.vectorValue(_t, _q_point[_qp]);
  return outer_product(diri_vel, diri_vel) * _normals[_qp] * _test[_i][_qp] * _rho[_qp];
}
