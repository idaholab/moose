//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChannelGradientBC.h"
#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", ChannelGradientBC);

InputParameters
ChannelGradientBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<VectorPostprocessorName>(
      "channel_gradient_pps", "The vector postprocessor name that holds the channel gradient.");
  MooseEnum axis_options("x y z");
  params.addRequiredParam<MooseEnum>(
      "axis", axis_options, "What coordinate axis this boundary lies on");
  params.addParam<MaterialPropertyName>("h_name", "h", "The name of the heat transfer coefficient");
  return params;
}

ChannelGradientBC::ChannelGradientBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _axis(getParam<MooseEnum>("axis")),
    _channel_gradient_axis_coordinate(getVectorPostprocessorValue("channel_gradient_pps", _axis)),
    _channel_gradient_value(getVectorPostprocessorValue("channel_gradient_pps", "gradient")),
    _h(getMaterialProperty<Real>("h_name"))
{
}

Real
ChannelGradientBC::getGradient()
{
  unsigned klo = 0;
  unsigned khi = _channel_gradient_axis_coordinate.size() - 1;
  Real r_int = _q_point[_qp](_axis);
  while (khi - klo > 1)
  {
    unsigned int k = (khi + klo) >> 1;
    if (_channel_gradient_axis_coordinate[k] > r_int)
      khi = k;
    else
      klo = k;
  }
  Real grad = _channel_gradient_value[klo] +
              (_channel_gradient_value[khi] - _channel_gradient_value[klo]) *
                  (r_int - _channel_gradient_axis_coordinate[klo]) /
                  (_channel_gradient_axis_coordinate[khi] - _channel_gradient_axis_coordinate[klo]);
  return grad;
}

Real
ChannelGradientBC::computeQpResidual()
{
  return _test[_i][_qp] * _h[_qp] * getGradient();
}

Real
ChannelGradientBC::computeQpJacobian()
{
  return 0.;
}
