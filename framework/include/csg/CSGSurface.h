//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

#include "MooseApp.h"
#include "CSGTransformation.h"

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
   */
  CSGSurface(const std::string & name, const std::string & surf_type);

  /**
   * Destructor
   */
  virtual ~CSGSurface() = default;

  /**
   * @brief Get the Surface Type
   *
   * @return type of surface
   */
  const std::string & getSurfaceType() const { return _surface_type; }

  /**
   * @brief Get the coefficients that define the surface
   *
   * @return map of coefficients and their values
   */
  virtual std::unordered_map<std::string, Real> getCoeffs() const = 0; // Pure virtual function

  /**
   * @brief given a point, determine its evaluation based on the surface equation. A positive value
   * indicates a point that lies in the positive half-space with regards to the surface, a negative
   * value indicates a point that lies in the negative side of the surface, and a value of 0
   * indicates that the point lies on the surface.
   *
   * @param p point
   * @return evaluation of point based on surface equation
   */
  virtual Real evaluateSurfaceEquationAtPoint(const Point & p) const = 0; // Pure virtual function

  /**
   * @brief given a point, determine if it is in the positive or negative
   * half-space for the surface
   *
   * @param p point
   * @return sign of the half-space
   */
  CSGSurface::Halfspace getHalfspaceFromPoint(const Point & p) const;

  /**
   * @brief Get the name of surface
   *
   * @return std::string name of surface
   */
  const std::string & getName() const { return _name; }

  /**
   * @brief apply a transformation to a surface
   *
   * @param type type of transformation to apply
   * @param values values for the transformation (3 values for any transformation type)
   */
  void applyTransformation(TransformationType type, const std::vector<Real> & values);

  /**
   * @brief Get the list of transformations applied to this surface
   *
   * @return const reference to the list of transformations
   */
  const std::vector<std::pair<TransformationType, std::vector<Real>>> & getTransformations() const
  {
    return _transformations;
  }

  /// Operator overload for checking if two CSGSurface objects are equal
  bool operator==(const CSGSurface & other) const;

  /// Operator overload for checking if two CSGSurface objects are not equal
  bool operator!=(const CSGSurface & other) const;

protected:
  /**
   * @brief Create clone of current surface, to be implemented by derived class
   *
   * @return unique_ptr to cloned surface
   */
  virtual std::unique_ptr<CSGSurface> clone() const = 0; // Pure virtual function

  // set the name of the surface - intentionally not public because
  // name needs to be managed at the CSGSurfaceList level
  void setName(const std::string & name) { _name = name; }

  /// Name of surface
  std::string _name;

  /// Type of surface that is being represented
  /// string is taken directly from the surface class name
  const std::string _surface_type;

  /// list of transformations applied to the surface (type, value) in the order they are applied
  std::vector<std::pair<TransformationType, std::vector<Real>>> _transformations;

  // CSGSurfaceList needs to be friend to access setName()
  friend class CSGSurfaceList;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGSurfaceTest, testSetName);
  ///@}
#endif
};
} // namespace CSG
