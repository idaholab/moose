//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionAnisotropicDragCoefficients.h"
#include "NS.h"

namespace nms = NS;

registerADMooseObject("NavierStokesApp", FunctionAnisotropicDragCoefficients);

defineADValidParams(
    FunctionAnisotropicDragCoefficients,
    AnisotropicDragCoefficients,
    params.addRequiredParam<std::vector<FunctionName>>(nms::cL, "Function linear (Darcy) drag coefficient");
    params.addRequiredParam<std::vector<FunctionName>>(nms::cQ, "Function quadratic (Forchheimer) drag coefficient");
    params.addClassDescription("Material providing function anisotropic drag coefficients"););

FunctionAnisotropicDragCoefficients::FunctionAnisotropicDragCoefficients(
    const InputParameters & parameters)
  : AnisotropicDragCoefficients(parameters),
    _speed(getADMaterialProperty<Real>(nms::speed)),
    _rho(getADMaterialProperty<Real>(nms::density)),
    _mu(getADMaterialProperty<Real>(nms::mu))
{
  std::vector<FunctionName> func_darcy = getParam<std::vector<FunctionName>>(nms::cL);
  std::vector<FunctionName> func_forch = getParam<std::vector<FunctionName>>(nms::cQ);

  if (func_darcy.size() != 3)
    paramError(nms::cL, "The length of the Darcy coefficient vector must be 3.");

  if (func_forch.size() != 3)
    paramError(nms::cQ, "The length of the Forchheimer coefficient vector must be 3.");

  _darcy_coefficient.resize(3);
  _forchheimer_coefficient.resize(3);

  for (unsigned int i = 0; i < 3; ++i)
  {
    _darcy_coefficient[i] = &getFunctionByName(func_darcy[i]);
    _forchheimer_coefficient[i] = &getFunctionByName(func_forch[i]);
  }
}

ADReal
FunctionAnisotropicDragCoefficients::computeDarcyCoefficient(const int & i)
{
  return _darcy_coefficient[i]->value(_t, _q_point[_qp]);
}

ADReal
FunctionAnisotropicDragCoefficients::computeForchheimerCoefficient(const int & i)
{
  return _forchheimer_coefficient[i]->value(_t, _q_point[_qp]);
}

ADReal
FunctionAnisotropicDragCoefficients::computeDarcyPrefactor()
{
  return _mu[_qp] / _rho[_qp];
}

ADReal
FunctionAnisotropicDragCoefficients::computeForchheimerPrefactor()
{
  return _speed[_qp];
}
