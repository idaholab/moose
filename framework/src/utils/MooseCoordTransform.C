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

using namespace libMesh;

MooseCoordTransform::Direction
MooseCoordTransform::processZAxis(const Direction z_axis)
{
  return _coord_type == Moose::COORD_RZ ? z_axis : INVALID;
}

void
MooseCoordTransform::setUpDirection(const Direction up_direction)
{
  Real alpha = 0, beta = 0, gamma = 0;

  const bool must_rotate_axes =
      _coord_type == Moose::COORD_RZ || _coord_type == Moose::COORD_RSPHERICAL;
  // Don't error immediately for unit testing purposes
  bool negative_radii = false;

  if (up_direction == X)
  {
    alpha = 90, beta = 0, gamma = 0;
    if (must_rotate_axes)
    {
      if (_r_axis == X)
      {
        _r_axis = Y;
        _z_axis = processZAxis(X);
      }
      else if (_r_axis == Y)
      {
        negative_radii = true;
        _r_axis = X;
        _z_axis = processZAxis(Y);
      }
      else
        mooseError("Bad r-axis value");
    }
  }
  else if (up_direction == Y)
    alpha = 0, beta = 0, gamma = 0;
  else if (up_direction == Z)
  {
    alpha = 0, beta = -90, gamma = 0;
    if (must_rotate_axes)
    {
      if (_r_axis == X)
      {
        _r_axis = X;
        _z_axis = processZAxis(Z);
      }
      else if (_r_axis == Y)
      {
        negative_radii = true;
        _r_axis = Z;
        _z_axis = processZAxis(X);
      }
      else
        mooseError("Bad r-axis value");
    }
  }
  else
    mooseError("Bad up direction value");

  _rotate = std::make_unique<RealTensorValue>(
      RealTensorValue::extrinsic_rotation_matrix(alpha, beta, gamma));
  computeRS();

  if (negative_radii)
    mooseError("Rotation yields negative radial values");
}

void
MooseCoordTransform::setRotation(const Real alpha, const Real beta, const Real gamma)
{
  const bool must_rotate_axes =
      _coord_type == Moose::COORD_RZ || _coord_type == Moose::COORD_RSPHERICAL;
  bool axes_rotated = false;
  if (must_rotate_axes)
  {
    const auto angles = std::make_tuple(alpha, beta, gamma);
    if (angles == std::make_tuple(0, 90, 0))
    {
      if (_r_axis == X)
      {
        mooseAssert((_coord_type == Moose::COORD_RZ && _z_axis == Y) ||
                        (_coord_type == Moose::COORD_RSPHERICAL && _z_axis == INVALID),
                    "'_z_axis' is not an expected value");
        _r_axis = X;
        _z_axis = _coord_type == Moose::COORD_RZ ? Z : INVALID;
      }
      else if (_r_axis == Y)
      {
        mooseAssert((_coord_type == Moose::COORD_RZ && _z_axis == X) ||
                        (_coord_type == Moose::COORD_RSPHERICAL && _z_axis == INVALID),
                    "'_z_axis' is not an expected value");
        _r_axis = Z;
        _z_axis = _coord_type == Moose::COORD_RZ ? X : INVALID;
      }
      axes_rotated = true;
    }
  }

  _rotate = std::make_unique<RealTensorValue>(
      RealTensorValue::extrinsic_rotation_matrix(alpha, beta, gamma));
  computeRS();

  if (must_rotate_axes && !axes_rotated)
    mooseError("Unsupported manual angle prescription in 'MooseCoordTransform::setRotation'");
}

void
MooseCoordTransform::setLengthUnit(const MooseUnits & length_unit)
{
  _length_unit = length_unit;
  const auto scale = Real(_length_unit / MooseUnits("m"));
  _scale =
      std::make_unique<RealTensorValue>(RealTensorValue(scale, 0, 0, 0, scale, 0, 0, 0, scale));
  computeRS();
}

void
MooseCoordTransform::setCoordinateSystem(const Moose::CoordinateSystemType coord_type,
                                         const Direction rz_symmetry_axis)
{
  _coord_type = coord_type;

  if (_coord_type == Moose::COORD_RZ)
  {
    if (rz_symmetry_axis == INVALID)
      mooseError("For RZ coordinate systems, the 'rz_symmetry_axis' parameter must be provided to "
                 "'MooseCoordTransform::setCoordinateSystem'");

    _z_axis = rz_symmetry_axis;
    _r_axis = _z_axis == X ? Y : X;
  }
  else if (_coord_type == Moose::COORD_RSPHERICAL)
    _r_axis = X;
}

InputParameters
MooseCoordTransform::validParams()
{
  auto params = emptyInputParameters();
  /// One entry of coord system per block, the size of _blocks and _coord_sys has to match, except:
  /// 1. _blocks.size() == 0, then there needs to be just one entry in _coord_sys, which will
  ///    be set for the whole domain
  /// 2. _blocks.size() > 0 and no coordinate system was specified, then the whole domain will be XYZ.
  /// 3. _blocks.size() > 0 and one coordinate system was specified, then the whole domain will be that system.
  params.addParam<std::vector<SubdomainName>>("block", "Block IDs for the coordinate systems");
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL", "XYZ");
  MooseEnum rz_coord_axis("X=0 Y=1", "Y");
  params.addParam<MultiMooseEnum>(
      "coord_type", coord_types, "Type of the coordinate system per block param");
  params.addParam<MooseEnum>(
      "rz_coord_axis", rz_coord_axis, "The rotation axis (X | Y) for axisymetric coordinates");
  params.addParam<std::string>(
      "length_unit",
      "How much distance one mesh length unit represents, e.g. 1 cm, 1 nm, 1 ft, 5inches");
  params.addRangeCheckedParam<Real>(
      "alpha_rotation",
      "-180<alpha_rotation<=180",
      "The number of degrees that the domain should be alpha-rotated using the Euler "
      "angle ZXZ convention from https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix in "
      "order to align with a canonical physical space of your choosing.");
  params.addRangeCheckedParam<Real>(
      "beta_rotation",
      "-180<beta_rotation<=180",
      "The number of degrees that the domain should be beta-rotated using the Euler "
      "angle ZXZ convention from https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix in "
      "order to align with a canonical physical space of your choosing.");
  params.addRangeCheckedParam<Real>(
      "gamma_rotation",
      "-180<gamma_rotation<=180",
      "The number of degrees that the domain should be gamma-rotated using the Euler "
      "angle ZXZ convention from https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix in "
      "order to align with a canonical physical space of your choosing.");
  MooseEnum up_direction("X=0 Y=1 Z=2");
  params.addParam<MooseEnum>(
      "up_direction",
      up_direction,
      "Specify what axis corresponds to the up direction in physical space (the opposite of the "
      "gravity vector if you will). If this parameter is provided, we will perform a single 90 "
      "degree rotation of the domain--if the provided axis is 'x' or 'z', we will not rotate if "
      "the axis is 'y'--such that a point which was on the provided axis will now lie on the "
      "y-axis, e.g. the y-axis is our canonical up direction. If you want finer grained control "
      "than this, please use the 'alpha_rotation', 'beta_rotation', and 'gamma_rotation' "
      "parameters.");
  return params;
}

MooseCoordTransform::MooseCoordTransform(const MooseMesh & mesh)
  : _translation(),
    _coord_type(Moose::COORD_XYZ),
    _r_axis(INVALID),
    _z_axis(INVALID),
    _destination_coord_type(Moose::COORD_XYZ),
    _destination_r_axis(INVALID),
    _destination_z_axis(INVALID),
    _has_different_coord_sys(false),
    _length_unit(std::string("1*m"))
{
  const auto & params = mesh.parameters();

  //
  // coordinate system transformation. If we have multiple different coordinate system types in our
  // problem, we take note of it because that can cause issues if there is a non-Cartesian
  // destination coordinate system
  //
  const auto & coord_sys = mesh.getCoordSystem();
  std::unordered_set<Moose::CoordinateSystemType> coord_types;
  auto map_it = coord_sys.begin();
  // It's possible that the mesh is not in a complete state
  if (map_it == coord_sys.end())
    setCoordinateSystem(Moose::COORD_XYZ);
  else
    setCoordinateSystem(
        map_it->second,
        Direction(static_cast<unsigned int>(int(params.get<MooseEnum>("rz_coord_axis")))));
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

  if (has_alpha || has_beta || has_gamma)
  {
    if (up_direction.isValid())
      mooseError("Cannot simultaneously set rotation angles as well as an up direction");

    const auto alpha = (has_alpha ? params.get<Real>("alpha_rotation") : Real(0));
    const auto beta = (has_beta ? params.get<Real>("beta_rotation") : Real(0));
    const auto gamma = (has_gamma ? params.get<Real>("gamma_rotation") : Real(0));

    setRotation(alpha, beta, gamma);
  }
  else if (up_direction.isValid())
    setUpDirection(Direction(static_cast<unsigned int>(int(up_direction))));

  //
  // Scale
  //
  if (params.isParamValid("length_unit"))
    setLengthUnit(MooseUnits(params.get<std::string>("length_unit")));
}

MooseCoordTransform::MooseCoordTransform()
  : _translation(),
    _coord_type(Moose::COORD_XYZ),
    _r_axis(INVALID),
    _z_axis(INVALID),
    _destination_coord_type(Moose::COORD_XYZ),
    _destination_r_axis(INVALID),
    _destination_z_axis(INVALID),
    _has_different_coord_sys(false),
    _length_unit(std::string("1*m"))
{
}

MooseCoordTransform::MooseCoordTransform(const MooseCoordTransform & other)
  : _translation(other._translation),
    _coord_type(other._coord_type),
    _r_axis(other._r_axis),
    _z_axis(other._z_axis),
    _destination_coord_type(other._destination_coord_type),
    _destination_r_axis(other._destination_r_axis),
    _destination_z_axis(other._destination_z_axis),
    _has_different_coord_sys(other._has_different_coord_sys),
    _length_unit(other._length_unit)
{
  if (other._scale)
    _scale = std::make_unique<RealTensorValue>(*other._scale);
  if (other._rotate)
    _rotate = std::make_unique<RealTensorValue>(*other._rotate);
  computeRS();
}

MooseCoordTransform &
MooseCoordTransform::operator=(const MooseCoordTransform & other)
{
  _translation = other._translation;
  _coord_type = other._coord_type;
  _r_axis = other._r_axis;
  _z_axis = other._z_axis;
  _destination_coord_type = other._destination_coord_type;
  _destination_r_axis = other._destination_r_axis;
  _destination_z_axis = other._destination_z_axis;
  _has_different_coord_sys = other._has_different_coord_sys;
  _length_unit = other._length_unit;

  if (other._scale)
    _scale = std::make_unique<RealTensorValue>(*other._scale);
  else
    _scale.reset();
  if (other._rotate)
    _rotate = std::make_unique<RealTensorValue>(*other._rotate);
  else
    _rotate.reset();

  computeRS();

  return *this;
}

void
MooseCoordTransform::computeRS()
{
  if (_scale || _rotate)
  {
    _rs = std::make_unique<RealTensorValue>(RealTensorValue(1, 0, 0, 0, 1, 0, 0, 0, 1));

    if (_scale)
      *_rs = *_scale * *_rs;
    if (_rotate)
      *_rs = *_rotate * *_rs;

    _rs_inverse = std::make_unique<RealTensorValue>(_rs->inverse());
  }
  else
  {
    _rs.reset();
    _rs_inverse.reset();
  }
}

Point
MooseCoordTransform::operator()(const Point & point) const
{
  Point ret(point);
  // Apply scaling and then rotation
  if (_rs)
    ret = (*_rs) * ret;

  // If this shows up in profiling we can make _translation a pointer
  ret += _translation;

  // Finally, coordinate system conversions
  if (_coord_type == Moose::COORD_XYZ && _destination_coord_type == Moose::COORD_RZ)
  {
    Real r_squared = 0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      if (i != _destination_z_axis)
        r_squared += ret(i) * ret(i);

    const auto r = std::sqrt(r_squared);
    const auto z = ret(_destination_z_axis);
    ret = 0;
    ret(_destination_r_axis) = r;
    ret(_destination_z_axis) = z;
  }
  else if (_coord_type == Moose::COORD_XYZ && _destination_coord_type == Moose::COORD_RSPHERICAL)
  {
    Real r_squared = 0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      r_squared += ret(i) * ret(i);

    const auto r = std::sqrt(r_squared);
    ret = 0;
    ret(_destination_r_axis) = r;
  }
  else if (_coord_type == Moose::COORD_RZ && _destination_coord_type == Moose::COORD_RSPHERICAL)
  {
    Real r_squared = 0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      mooseAssert(
          i == _r_axis || i == _z_axis || MooseUtils::absoluteFuzzyEqual(ret(i), 0),
          "Our point should be 0 if we are evaluating at an index that is neither our r or z-axis");
      r_squared += ret(i) * ret(i);
    }

    const auto r = std::sqrt(r_squared);
    ret = 0;
    ret(_destination_r_axis) = r;
  }

  return ret;
}

Point
MooseCoordTransform::mapBack(const Point & point) const
{
  Point ret(point);

  // inverse translate
  ret -= _translation;

  // inverse rotate and then inverse scale
  if (_rs_inverse)
    ret = (*_rs_inverse) * ret;

  // Finally, coordinate system conversions
  if ((_coord_type == Moose::COORD_XYZ && (_destination_coord_type == Moose::COORD_RZ ||
                                           _destination_coord_type == Moose::COORD_RSPHERICAL)) ||
      (_coord_type == Moose::COORD_RZ && _destination_coord_type == Moose::COORD_RSPHERICAL))
    mooseError("Coordinate collapsing occurred in going to the reference space. There is no unique "
               "return mapping");

  return ret;
}

void
MooseCoordTransform::setTranslationVector(const Point & translation)
{
  _translation = translation;
}

void
MooseCoordTransform::setDestinationCoordinateSystem(
    const MooseCoordTransform & destination_coord_transform)
{
  _destination_coord_type = destination_coord_transform._coord_type;
  _destination_r_axis = destination_coord_transform._r_axis;
  _destination_z_axis = destination_coord_transform._z_axis;

  if (destination_coord_transform._has_different_coord_sys &&
      (_has_different_coord_sys || _coord_type != Moose::COORD_RSPHERICAL))
    mooseError(
        "The destination coordinate system has different coordinate systems and we have coordinate "
        "system(s) that could require coordinate collapsing when transforming from our coordinate "
        "system to the destination coordinate system. Because our transform method only takes a "
        "point argument, and not subdomain arguments, the transform is ambiguous");

  if ((_destination_coord_type == Moose::COORD_RZ ||
       _destination_coord_type == Moose::COORD_RSPHERICAL) &&
      _has_different_coord_sys)
    mooseError("When the destination coordinate system is RZ or RSPHERICAL, we have to perform "
               "coordinate collapsing based on *our* coordinate system. However, we have multiple "
               "coordinate systems, and since when evaluating transformations, we are only "
               "called with a Point argument, we do not know what subdomain we are on and "
               "consequently we do not know what coordinate collapse to apply.");
}

bool
MooseCoordTransform::hasNonTranslationTransformation() const
{
  if (_rs)
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
      {
        const auto matrix_elem = (*_rs)(i, j);
        if (i == j)
        {
          if (!MooseUtils::absoluteFuzzyEqual(matrix_elem, 1))
            return true;
        }
        else if (!MooseUtils::absoluteFuzzyEqual(matrix_elem, 0))
          return true;
      }

  if ((_coord_type == Moose::COORD_XYZ && (_destination_coord_type == Moose::COORD_RZ ||
                                           _destination_coord_type == Moose::COORD_RSPHERICAL)) ||
      (_coord_type == Moose::COORD_RZ && _destination_coord_type == Moose::COORD_RSPHERICAL))
    return true;

  return false;
}

bool
MooseCoordTransform::isIdentity() const
{
  if (hasNonTranslationTransformation())
    return false;

  for (const auto i : make_range(Moose::dim))
    if (!MooseUtils::absoluteFuzzyEqual(_translation(i), 0))
      return false;

  return true;
}
