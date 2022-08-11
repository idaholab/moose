//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteLineSegmentInterface.h"
#include "MooseObject.h"
#include "MooseEnum.h"
#include "MooseUtils.h"

InputParameters
DiscreteLineSegmentInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<Point>("position", "Start position of axis in 3-D space [m]");
  params.addRequiredParam<RealVectorValue>(
      "orientation",
      "Direction of axis from start position to end position (no need to normalize)");
  params.addParam<Real>("rotation", 0.0, "Angle of rotation about the x-axis [degrees]");
  params.addRequiredParam<std::vector<Real>>("length", "Length of each axial section [m]");
  params.addRequiredParam<std::vector<unsigned int>>("n_elems",
                                                     "Number of elements in each axial section");

  return params;
}

DiscreteLineSegmentInterface::DiscreteLineSegmentInterface(const MooseObject * moose_object)
  : _position(moose_object->parameters().get<Point>("position")),
    _dir_unnormalized(moose_object->parameters().get<RealVectorValue>("orientation")),
    _dir(_dir_unnormalized / _dir_unnormalized.norm()),
    _rotation(moose_object->parameters().get<Real>("rotation")),
    _lengths(moose_object->parameters().get<std::vector<Real>>("length")),
    _length(std::accumulate(_lengths.begin(), _lengths.end(), 0.0)),
    _n_elems(moose_object->parameters().get<std::vector<unsigned int>>("n_elems")),
    _n_elem(std::accumulate(_n_elems.begin(), _n_elems.end(), 0)),
    _n_sections(_lengths.size()),
    _R(computeDirectionTransformationTensor()),
    _Rx(computeXRotationTransformationTensor()),
    _R_inv(_R.inverse()),
    _Rx_inv(_Rx.inverse()),
    _moose_object_name_dlsi(moose_object->name())
{
}

RealTensorValue
DiscreteLineSegmentInterface::computeDirectionTransformationTensor() const
{
  Real theta = acos(_dir(2));
  Real aphi = atan2(_dir(1), _dir(0));
  return RealTensorValue(
      RealVectorValue(cos(aphi) * sin(theta), -sin(aphi), -cos(aphi) * cos(theta)),
      RealVectorValue(sin(aphi) * sin(theta), cos(aphi), -sin(aphi) * cos(theta)),
      RealVectorValue(cos(theta), 0.0, sin(theta)));
}

RealTensorValue
DiscreteLineSegmentInterface::computeXRotationTransformationTensor() const
{
  Real rotation_rad = M_PI * _rotation / 180.;
  RealVectorValue Rx_x(1., 0., 0.);
  RealVectorValue Rx_y(0., cos(rotation_rad), -sin(rotation_rad));
  RealVectorValue Rx_z(0., sin(rotation_rad), cos(rotation_rad));
  return RealTensorValue(Rx_x, Rx_y, Rx_z);
}

Real
DiscreteLineSegmentInterface::computeAxialCoordinate(const Point & p) const
{
  const Real ax_coord = _dir * (p - _position);
  if (MooseUtils::absoluteFuzzyLessThan(ax_coord, 0.0) ||
      MooseUtils::absoluteFuzzyGreaterThan(ax_coord, _length))
    mooseError(_moose_object_name_dlsi,
               ": The point ",
               p,
               " has an invalid axial coordinate (",
               ax_coord,
               "). Valid axial coordinates are in the range (0,",
               _length,
               ").");
  else
    return ax_coord;
}

Point
DiscreteLineSegmentInterface::computeRealPointFromReferencePoint(const Point & p) const
{
  return _R * (_Rx * p) + _position;
}

Point
DiscreteLineSegmentInterface::computeReferencePointFromRealPoint(const Point & p) const
{
  return _Rx_inv * _R_inv * (p - _position);
}
