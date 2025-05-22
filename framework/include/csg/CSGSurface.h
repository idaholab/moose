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
  /// An enum for type of surface that is being represented
  enum class SurfaceType
  {
    INVALID,
    PLANE,
    SPHERE,
    XCYLINDER,
    YCYLINDER,
    ZCYLINDER
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
   *
   * @param name unique name of surface
   */
  CSGSurface(const std::string name);

  /**
   * @brief Construct a new CSGSurface
   *
   * @param name unique name of surface
   * @param surf_type surface type
   */
  CSGSurface(const std::string name, SurfaceType surf_type);

  /**
   * Destructor
   */
  virtual ~CSGSurface() = default;

  /**
   * @brief Get the Surface Type (i.e. PLANE, SPHERE, XCYLINDER, YCYLINDER, ZCYLINDER)
   *
   * @return SurfaceType type of surface
   */
  SurfaceType getSurfaceType() const { return _surface_type; }

  /**
   * @brief Get the string representation of surface type
   *
   * @return const std::string string representation of surface type
   */
  const std::string getSurfaceTypeString();

  /**
   * @brief Set the Boundary Type (i.e. transmission or vacuum)
   *
   * @param boundary_type type of boundary
   */
  void setBoundaryType(const BoundaryType boundary_type) { _boundary_type = boundary_type; }

  /**
   * @brief Get the Boundary Type (i.e. transmission or vacuum)
   *
   * @return BoundaryType type of boundary
   */
  BoundaryType getBoundaryType() const { return _boundary_type; }

  /**
   * @brief Get the string representation of Boundary Type
   *
   * @return const std::string string representation of the boundary type
   */
  const std::string getBoundaryTypeString();

  /**
   * @brief Get the coefficients that define the surface
   *
   * @return std::map<std::string, Real> map of coefficients and their values
   */
  virtual std::map<std::string, Real> getCoeffs() = 0; // Pure virtual function

  /**
   * @brief get direction from point to surface
   *
   * @param p point
   * @return CSGSurface::Direction
   */
  virtual CSGSurface::Direction directionFromPoint(const Point p) = 0; // Pure virtual function

  /**
   * @brief Get the name of surface
   *
   * @return std::string name of surface
   */
  std::string getName() const { return _name; }

protected:
  /// Name of surface
  std::string _name;

  /// Type of surface that is being represented
  SurfaceType _surface_type;

  /// Boundary type of surface
  BoundaryType _boundary_type;

  // set the name of the surface - intentionally not public because
  // name needs to be managed at the CSGSurfaceList level
  void setName(const std::string name) { _name = name; }

  // CSGSurfaceList needs to be friend to access setName()
  friend class CSGSurfaceList;
};
} // namespace CSG
