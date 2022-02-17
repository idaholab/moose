//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseCoordTransform.h"
#include "InputParameters.h"
#include "MultiMooseEnum.h"
#include "MooseEnum.h"
#include "MooseMesh.h"

#include "libmesh/tensor_value.h"
#include "libmesh/point.h"

using namespace libMesh;

MooseCoordTransform::MooseCoordTransform() {}
MooseCoordTransform::~MooseCoordTransform() {}

void
MooseCoordTransform::setRotation(const Direction up_direction, const Direction rz_z_axis /*=Y*/)
{
  Real alpha = 0, beta = 0, gamma = 0;
  if (rz_z_axis == Z)
    mooseError("Invalid rz_coord_axis option of 'Z'");
  else if (rz_z_axis == INVALID)
    mooseError("Called MooseCoordTransform::setRotation with INVALID 'rz_z_axis'");

  const Direction rz_r_axis = rz_z_axis == X ? Y : X;

  // Don't error immediately for unit testing purposes
  bool bad_rz_rotation = false;

  if (up_direction == X)
  {
    alpha = 90, beta = 0, gamma = 0;
    if (rz_r_axis == X)
    {
      _rz_r_axis = Y;
      _rz_z_axis = X;
    }
    else if (rz_r_axis == Y)
    {
      bad_rz_rotation = true;
      _rz_r_axis = X;
      _rz_z_axis = Y;
    }
    else
      mooseAssert(false, "we should never get here");
  }
  else if (up_direction == Y)
  {
    alpha = 0, beta = 0, gamma = 0;
    _rz_r_axis = rz_r_axis;
    _rz_z_axis = rz_z_axis;
  }
  else if (up_direction == Z)
  {
    alpha = 0, beta = -90, gamma = 0;
    if (rz_r_axis == X)
    {
      _rz_r_axis = X;
      _rz_z_axis = Z;
    }
    else if (rz_r_axis == Y)
    {
      bad_rz_rotation = true;
      _rz_r_axis = Z;
      _rz_z_axis = X;
    }
    else
      mooseAssert(false, "we should never get here");
  }
  else
    mooseAssert(false, "we should never get here");

  setRotation(alpha, beta, gamma);

  if (bad_rz_rotation)
    mooseError("Rotation yields negative radial values");
}

void
MooseCoordTransform::setRotation(const Real alpha, const Real beta, const Real gamma)
{
  _rotate = std::make_unique<RealTensorValue>(
      RealTensorValue::extrinsic_rotation_matrix(alpha, beta, gamma));
}

void
MooseCoordTransform::setRotation(const Real alpha,
                                 const Real beta,
                                 const Real gamma,
                                 const Direction rz_z_axis)
{
  if (rz_z_axis == Z)
    mooseError("Invalid rz_coord_axis option of 'Z'");
  else if (rz_z_axis == INVALID)
    mooseError("Called MooseCoordTransform::setRotation with INVALID 'rz_z_axis'");

  const Direction rz_r_axis = rz_z_axis == X ? Y : X;

  bool supported_manual_rz_rotation = false;
  const auto angles = std::make_tuple(alpha, beta, gamma);
  if (angles == std::make_tuple(0, 90, 0))
  {
    if (rz_r_axis == X)
    {
      _rz_r_axis = X;
      _rz_z_axis = Z;
    }
    else if (rz_r_axis == Y)
    {
      _rz_r_axis = Z;
      _rz_z_axis = X;
    }
    supported_manual_rz_rotation = true;
  }

  _rotate = std::make_unique<RealTensorValue>(
      RealTensorValue::extrinsic_rotation_matrix(alpha, beta, gamma));

  if (!supported_manual_rz_rotation)
    mooseError("Unsupported manual angle prescription in 'MooseCoordTransform::setRotation'");
}

void
MooseCoordTransform::setLengthUnitsPerMeter(const Real length_units_per_meter)
{
  const auto scale = 1 / length_units_per_meter;

  _scale =
      std::make_unique<RealTensorValue>(RealTensorValue(scale, 0, 0, 0, scale, 0, 0, 0, scale));
}

void
MooseCoordTransform::setCoordinateSystem(const Moose::CoordinateSystemType coord_type)
{
  _coord_type = coord_type;
}

MooseCoordTransform::MooseCoordTransform(const InputParameters & params)
{
  //
  // coordinate system transformation. If we have multiple different coordinate system types in our
  // problem, we take note of it because that can cause issues if there is a non-Cartesian
  // destination coordinate system
  //
  const auto & mesh = *params.getCheckedPointerParam<MooseMesh *>("mesh");
  const auto & coord_sys = mesh.getCoordSystem();
  std::unordered_set<Moose::CoordinateSystemType> coord_types;
  auto map_it = coord_sys.begin();
  // It's possible that the mesh is not in a complete state
  if (map_it == coord_sys.end())
    setCoordinateSystem(Moose::COORD_XYZ);
  else
    setCoordinateSystem(map_it->second);
  for (; map_it != coord_sys.end(); ++map_it)
    coord_types.insert(map_it->second);

  _has_different_coord_sys = coord_types.size() > 1;

  //
  // rotation
  //
  const bool has_alpha = params.isParamValid("alpha_rotation");
  const bool has_beta = params.isParamValid("beta_rotation");
  const bool has_gamma = params.isParamValid("gamma_rotation");
  const auto & up_direction = params.get<MooseEnum>("up_direction");

  const Direction rz_z_axis =
      (_coord_type == Moose::COORD_RZ)
          ? Direction(static_cast<unsigned int>(int(params.get<MooseEnum>("rz_coord_axis"))))
          : Y;

  if (has_alpha || has_beta || has_gamma)
  {
    if (up_direction.isValid())
      mooseError("Cannot simultaneously set rotation angles as well as an up direction");

    const auto alpha = (has_alpha ? params.get<Real>("alpha_rotation") : Real(0));
    const auto beta = (has_beta ? params.get<Real>("beta_rotation") : Real(0));
    const auto gamma = (has_gamma ? params.get<Real>("gamma_rotation") : Real(0));

    if (_coord_type == Moose::COORD_RZ)
      setRotation(alpha, beta, gamma, rz_z_axis);
    else
      setRotation(alpha, beta, gamma);
  }
  else if (up_direction.isValid())
    setRotation(Direction(static_cast<unsigned int>(int(up_direction))), rz_z_axis);

  //
  // Scale
  //
  if (params.isParamValid("length_units_per_meter"))
    setLengthUnitsPerMeter(params.get<Real>("length_units_per_meter"));
}

Point
MooseCoordTransform::operator()(const Point & point) const
{
  Point ret(point);
  if (_rotate)
    ret = (*_rotate) * ret;
  if (_scale)
    ret = (*_scale) * ret;

  // If this shows up in profiling we can make _translation a pointer
  ret += _translation;

  // Finally, coordinate system conversions
  if (_coord_type == Moose::COORD_XYZ && _destination_coord_type == Moose::COORD_RZ)
  {
    Real r_squared = 0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      if (i != _destination_rz_z_axis)
        r_squared += ret(i) * ret(i);

    const auto r = std::sqrt(r_squared);
    const auto z = ret(_destination_rz_z_axis);
    ret = 0;
    ret(_destination_rz_r_axis) = r;
    ret(_destination_rz_z_axis) = z;
  }

  return ret;
}

void
MooseCoordTransform::setTranslationVector(const Point & translation)
{
  _translation = translation;
}

void
MooseCoordTransform::setDestinationCoordinateSystem(
    const Moose::CoordinateSystemType destination_coord_type,
    const Direction rz_r_axis,
    const Direction rz_z_axis)
{
  _destination_coord_type = destination_coord_type;
  if (_destination_coord_type == Moose::COORD_RZ)
  {
    auto check_axes = [](const auto & axis, const auto & axis_string)
    {
      if (axis == INVALID)
        mooseError("If the destination coordinate system type is RZ, then a valid '",
                   axis_string,
                   "' must be provided to 'MooseCoordTransform::setDestinationCoordinateSystem'");
    };

    check_axes(rz_r_axis, "rz_r_axis");
    check_axes(rz_z_axis, "rz_z_axis");

    if (_has_different_coord_sys)
      mooseError("When the destination coordinate system is RZ, we may have to perform "
                 "transformations based on *our* coordinate system. However, we have multiple "
                 "coordinate systems, and since when evaluating transformations, we are only "
                 "called with a Point argument, we do not know what subdomain we are on and "
                 "consequently we do not know what transformation to apply.");
  }

  _destination_rz_r_axis = rz_r_axis;
  _destination_rz_z_axis = rz_z_axis;
}
