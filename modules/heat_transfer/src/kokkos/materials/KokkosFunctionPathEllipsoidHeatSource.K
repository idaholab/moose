//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionPathEllipsoidHeatSource.h"

registerKokkosMaterial("HeatTransferApp", KokkosFunctionPathEllipsoidHeatSource);

InputParameters
KokkosFunctionPathEllipsoidHeatSource::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<Real>("power", "power");
  params.addParam<Real>("efficiency", 1, "process efficiency");
  params.addRequiredParam<Real>("rx", "effective transverse ellipsoid radius");
  params.addRequiredParam<Real>("ry", "effective longitudinal ellipsoid radius");
  params.addRequiredParam<Real>("rz", "effective depth ellipsoid radius");
  params.addParam<Real>(
      "factor", 1, "scaling factor that is multiplied to the heat source to adjust the intensity");
  params.addParam<FunctionName>(
      "function_x", "0", "The x component of the center of the heating spot as a function of time");
  params.addParam<FunctionName>(
      "function_y", "0", "The y component of the center of the heating spot as a function of time");
  params.addParam<FunctionName>(
      "function_z", "0", "The z component of the center of the heating spot as a function of time");
  params.addClassDescription("Double ellipsoid volumetric source heat with function path.");
  return params;
}

KokkosFunctionPathEllipsoidHeatSource::KokkosFunctionPathEllipsoidHeatSource(
    const InputParameters & parameters)
  : Material(parameters),
    _P(getParam<Real>("power")),
    _eta(getParam<Real>("efficiency")),
    _rx(getParam<Real>("rx")),
    _ry(getParam<Real>("ry")),
    _rz(getParam<Real>("rz")),
    _f(getParam<Real>("factor")),
    _function_x(getKokkosFunction("function_x")),
    _function_y(getKokkosFunction("function_y")),
    _function_z(getKokkosFunction("function_z")),
    _volumetric_heat(declareKokkosProperty<Real>("volumetric_heat"))
{
}
