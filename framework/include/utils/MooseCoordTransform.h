//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "libmesh/point.h"
#include <memory>

class InputParameters;
namespace libMesh
{
template <typename>
class TensorValue;
typedef TensorValue<Real> RealTensorValue;
}

class MooseCoordTransform
{
public:
  MooseCoordTransform();
  MooseCoordTransform(const InputParameters & params);
  ~MooseCoordTransform();

  libMesh::Point operator()(const libMesh::Point & point) const;

  /**
   * Set how much this coordinate system origin is translated from the canonical/reference
   * coordinate system origin. The translation vector itself should be in reference frame
   * coordinates, e.g. if this API is being used to set the translation of a multi-app based on the
   * multi-app parameter \p positions, then a point from \p positions should be passed to the main
   * application's \p MooseCoordTransform::operator() in order to get the translation in the
   * reference frame
   */
  void setTranslationVector(const libMesh::Point & translation);

  enum Direction : unsigned int
  {
    X = 0,
    Y,
    Z,
    INVALID
  };

  void setDestinationCoordinateSystem(Moose::CoordinateSystemType destination_coord_type,
                                      Direction rz_r_axis = INVALID,
                                      Direction rz_z_axis = INVALID);

  Moose::CoordinateSystemType coordinateSystem() const { return _coord_type; }
  Direction rzRAxis() const { return _rz_r_axis; }
  Direction rzZAxis() const { return _rz_z_axis; }

  void setupFromParameters(const InputParameters & params);
  void setRotation(Direction up_direction, Direction rz_z_axis = Y);
  void setRotation(Real alpha, Real beta, Real gamma);
  void setRotation(Real alpha, Real beta, Real gamma, Direction rz_z_axis);
  void setLengthUnitsPerMeter(Real length_units_per_meter);
  void setCoordinateSystem(Moose::CoordinateSystemType);

private:
  std::unique_ptr<libMesh::RealTensorValue> _scale;
  std::unique_ptr<libMesh::RealTensorValue> _rotate;
  libMesh::Point _translation;
  Moose::CoordinateSystemType _coord_type = Moose::COORD_XYZ;
  Direction _rz_r_axis = INVALID;
  Direction _rz_z_axis = INVALID;

  Moose::CoordinateSystemType _destination_coord_type = Moose::COORD_XYZ;
  Direction _destination_rz_r_axis = INVALID;
  Direction _destination_rz_z_axis = INVALID;

  bool _has_different_coord_sys = false;
};
