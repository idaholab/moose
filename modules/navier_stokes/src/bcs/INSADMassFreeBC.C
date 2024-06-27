//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADMassFreeBC.h"

registerMooseObject("NavierStokesApp", INSADMassFreeBC);

InputParameters
INSADMassFreeBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Specifies flow of mass through a boundary given a velocity function or postprocessor");
  params.addRequiredCoupledVar("velocity", "The velocity");
  params.addParam<FunctionName>("v_fn", "Velocity function with time at the boundary");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
  return params;
}

INSADMassFreeBC::INSADMassFreeBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _velocity(adCoupledVectorValue("velocity")),
    _has_vfn(parameters.isParamValid("v_fn")),
    _velocity_fn(_has_vfn ? &getFunction("v_fn") : NULL),
    _rho(getADMaterialProperty<Real>("rho_name"))
{
}

ADReal
INSADMassFreeBC::computeQpResidual()
{
  if (_has_vfn) {
    ADRealVectorValue v_bc = _velocity_fn->value(_t, _q_point[_qp]);
    return -v_bc * _test[_i][_qp] * _normals[_qp];
  }

  return -_velocity[_qp] * _test[_i][_qp] * _normals[_qp];
}

