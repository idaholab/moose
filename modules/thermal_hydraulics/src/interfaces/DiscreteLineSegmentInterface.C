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
#include "Numerics.h"

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
    _R(computeDirectionTransformationTensor(_dir)),
    _Rx(computeXRotationTransformationTensor(_rotation)),
    _R_inv(_R.inverse()),
    _Rx_inv(_Rx.inverse()),
    _moose_object_name_dlsi(moose_object->name())
{
}

RealTensorValue
DiscreteLineSegmentInterface::computeDirectionTransformationTensor(const RealVectorValue & dir)
{
  const auto dir_normalized = dir / dir.norm();
  const Real theta = acos(dir_normalized(2));
  const Real aphi = atan2(dir_normalized(1), dir_normalized(0));
  return RealTensorValue(
      RealVectorValue(cos(aphi) * sin(theta), -sin(aphi), -cos(aphi) * cos(theta)),
      RealVectorValue(sin(aphi) * sin(theta), cos(aphi), -sin(aphi) * cos(theta)),
      RealVectorValue(cos(theta), 0.0, sin(theta)));
}

RealTensorValue
DiscreteLineSegmentInterface::computeXRotationTransformationTensor(const Real & rotation)
{
  const Real rotation_rad = M_PI * rotation / 180.;
  const RealVectorValue Rx_x(1., 0., 0.);
  const RealVectorValue Rx_y(0., cos(rotation_rad), -sin(rotation_rad));
  const RealVectorValue Rx_z(0., sin(rotation_rad), cos(rotation_rad));
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
DiscreteLineSegmentInterface::computeRealPointFromReferencePoint(const Point & p,
                                                                 const RealVectorValue & position,
                                                                 const RealTensorValue & R,
                                                                 const RealTensorValue & Rx)
{
  return R * (Rx * p) + position;
}

Point
DiscreteLineSegmentInterface::computeRealPointFromReferencePoint(const Point & p) const
{
  return computeRealPointFromReferencePoint(p, _position, _R, _Rx);
}

Point
DiscreteLineSegmentInterface::computeReferencePointFromRealPoint(const Point & p,
                                                                 const RealVectorValue & position,
                                                                 const RealTensorValue & R_inv,
                                                                 const RealTensorValue & Rx_inv)
{
  return Rx_inv * R_inv * (p - position);
}

Point
DiscreteLineSegmentInterface::computeReferencePointFromRealPoint(const Point & p) const
{
  return computeReferencePointFromRealPoint(p, _position, _R_inv, _Rx_inv);
}

MooseEnum
DiscreteLineSegmentInterface::getAlignmentAxis(const RealVectorValue & dir)
{
  MooseEnum axis("x y z");
  if (THM::areParallelVectors(dir, RealVectorValue(1, 0, 0)))
    axis = "x";
  else if (THM::areParallelVectors(dir, RealVectorValue(0, 1, 0)))
    axis = "y";
  else if (THM::areParallelVectors(dir, RealVectorValue(0, 0, 1)))
    axis = "z";

  return axis;
}

MooseEnum
DiscreteLineSegmentInterface::getAlignmentAxis() const
{
  return getAlignmentAxis(_dir);
}

std::vector<Real>
DiscreteLineSegmentInterface::getElementBoundaryCoordinates(
    const RealVectorValue & position,
    const RealVectorValue & orientation,
    const Real & rotation,
    const std::vector<Real> & lengths,
    const std::vector<unsigned int> & n_elems)
{
  const auto axis = getAlignmentAxis(orientation);

  unsigned int d;
  if (axis == "x")
    d = 0;
  else if (axis == "y")
    d = 1;
  else if (axis == "z")
    d = 2;
  else
    mooseError("Invalid axis.");

  const auto R = computeDirectionTransformationTensor(orientation);
  const auto Rx = computeXRotationTransformationTensor(rotation);

  const unsigned int n_elems_total = std::accumulate(n_elems.begin(), n_elems.end(), 0);
  const unsigned int n_sections = lengths.size();

  unsigned int i_section_start = 0;
  Real section_start_ref_position = 0.0;
  std::vector<Real> element_boundary_coordinates(n_elems_total + 1);
  element_boundary_coordinates[0] =
      computeRealPointFromReferencePoint(Point(0, 0, 0), position, R, Rx)(d);
  for (unsigned int j = 0; j < n_sections; ++j)
  {
    const Real section_dx = lengths[j] / n_elems[j];
    for (unsigned int k = 0; k < n_elems[j]; ++k)
    {
      const unsigned int i = i_section_start + k + 1;
      const Real ref_position = section_start_ref_position + section_dx * k;
      const Point ref_point = Point(ref_position, 0, 0);
      element_boundary_coordinates[i] =
          computeRealPointFromReferencePoint(ref_point, position, R, Rx)(d);
    }

    section_start_ref_position += lengths[j];
    i_section_start += n_elems[j];
  }

  return element_boundary_coordinates;
}

std::vector<Real>
DiscreteLineSegmentInterface::getElementBoundaryCoordinates() const
{
  return getElementBoundaryCoordinates(_position, _dir, _rotation, _lengths, _n_elems);
}
