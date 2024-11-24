//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

namespace CSG
{

/**
 * CSGSurface creates an internal representation of a Constructive Solid Geometry (CSG)
 * surface, represented as some polynomial in x, y, and z
 */
class CSGSurface
{
public:
  /// An enum for type of surface that is being represent
  enum class SurfaceType
  {
    invalid,
    plane
  };

  /// An enum for boundary type of surface
  enum class BoundaryType
  {
    transmission,
    vacuum
  };

  /// Enum for the direction of the halfspace being represented by a point and surface
  enum class Direction
  {
    positive,
    negative
  };

  /**
   * Default constructor
   */
  CSGSurface(const std::string name);

  CSGSurface(const std::string name, SurfaceType surf_type);

  /**
   * Destructor
   */
  virtual ~CSGSurface() = default;

  SurfaceType getSurfaceType() const { return _surface_type; }

  void setBoundaryType(const BoundaryType boundary_type) { _boundary_type = boundary_type; }

  BoundaryType getBoundaryType() const { return _boundary_type; }

  virtual std::map<std::string, Real> getCoeffs() = 0; // Pure virtual function

  virtual CSGSurface::Direction directionFromPoint(const Point p) = 0; // Pure virtual function

  std::string getName() const { return _name; }

protected:
  /// Name of surface
  std::string _name;

  /// Type of surface that is being represented
  SurfaceType _surface_type;

  /// Boundary type of surface
  BoundaryType _boundary_type;
};
} // namespace CSG
