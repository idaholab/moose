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
#include "MooseEnum.h"

namespace CSG
{

/**
 * CSGSurface creates an internal representation of a Constructive Solid Geometry (CSG)
 * surface, represented as some polynomial in x, y, and z
 */
class CSGSurface
{
public:
  /// Enum for the sign of the half-space being represented by a point and surface
  enum class Halfspace
  {
    POSITIVE,
    NEGATIVE
  };

  /**
   * Default constructor
   *
   * @param name unique name of surface
   */
  CSGSurface(const std::string & name);

  /**
   * @brief Construct a new CSGSurface
   *
   * @param name unique name of surface
   * @param surf_type surface type
   * @param boundary boundary condition for the surface
   */
  CSGSurface(const std::string & name, const std::string & surf_type, std::string & boundary);

  /**
   * Destructor
   */
  virtual ~CSGSurface() = default;

  /**
   * @brief Get the Surface Type
   *
   * @return type of surface
   */
  const std::string getSurfaceType() const { return _surface_type; }

  /**s
   * @brief Set the Boundary Type (i.e. TRANSMISSION or VACUUM)
   *
   * @param boundary_type type of boundary
   */
  void setBoundaryType(std::string boundary_type) { _boundary_type = boundary_type; }

  /**
   * @brief Get the Boundary Type (i.e. TRANSMISSION or VACUUM)
   *
   * @return type of boundary
   */
  std::string getBoundaryType() const { return _boundary_type; }

  /**
   * @brief Get the coefficients that define the surface
   *
   * @return map of coefficients and their values
   */
  virtual std::unordered_map<std::string, Real> getCoeffs() const = 0; // Pure virtual function

  /**
   * @brief given a point, determine if it is in the positive or negative
   * half-space for the surface
   *
   * @param p point
   * @return sign of the half-space
   */
  virtual CSGSurface::Halfspace
  getHalfspaceFromPoint(const Point & p) const = 0; // Pure virtual function

  /**
   * @brief Get the name of surface
   *
   * @return std::string name of surface
   */
  const std::string & getName() const { return _name; }

  /// Operator overload for checking if two CSGSurface objects are equal
  bool operator==(const CSGSurface & other) const;

  /// Operator overload for checking if two CSGSurface objects are not equal
  bool operator!=(const CSGSurface & other) const;

protected:
  // set the name of the surface - intentionally not public because
  // name needs to be managed at the CSGSurfaceList level
  void setName(const std::string & name) { _name = name; }

  /// Name of surface
  std::string _name;

  /// Type of surface that is being represented
  /// string is taken directly from the surface class name
  const std::string _surface_type;

  /// An enum for boundary type of surface
  MooseEnum _boundary_type;

  // CSGSurfaceList needs to be friend to access setName()
  friend class CSGSurfaceList;
};
} // namespace CSG
