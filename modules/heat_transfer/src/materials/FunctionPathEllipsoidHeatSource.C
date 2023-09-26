//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionPathEllipsoidHeatSource.h"

#include "Function.h"

registerMooseObject("HeatConductionApp", FunctionPathEllipsoidHeatSource);

InputParameters
FunctionPathEllipsoidHeatSource::validParams()
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

FunctionPathEllipsoidHeatSource::FunctionPathEllipsoidHeatSource(const InputParameters & parameters)
  : Material(parameters),
    _P(getParam<Real>("power")),
    _eta(getParam<Real>("efficiency")),
    _rx(getParam<Real>("rx")),
    _ry(getParam<Real>("ry")),
    _rz(getParam<Real>("rz")),
    _f(getParam<Real>("factor")),
    _function_x(getFunction("function_x")),
    _function_y(getFunction("function_y")),
    _function_z(getFunction("function_z")),
    _volumetric_heat(declareADProperty<Real>("volumetric_heat"))
{
}

void
FunctionPathEllipsoidHeatSource::computeQpProperties()
{
  const Real & x = _q_point[_qp](0);
  const Real & y = _q_point[_qp](1);
  const Real & z = _q_point[_qp](2);

  // center of the heat source
  Real x_t = _function_x.value(_t);
  Real y_t = _function_y.value(_t);
  Real z_t = _function_z.value(_t);

  _volumetric_heat[_qp] = 6.0 * std::sqrt(3.0) * _P * _eta * _f /
                          (_rx * _ry * _rz * std::pow(libMesh::pi, 1.5)) *
                          std::exp(-(3.0 * std::pow(x - x_t, 2.0) / std::pow(_rx, 2.0) +
                                     3.0 * std::pow(y - y_t, 2.0) / std::pow(_ry, 2.0) +
                                     3.0 * std::pow(z - z_t, 2.0) / std::pow(_rz, 2.0)));
}
