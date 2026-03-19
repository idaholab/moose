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
#include "MooseEnum.h"

namespace CSG
{

/**
 * Enumeration of transformation types that can be applied to CSG objects.
 */
enum class TransformationType
{
  TRANSLATION = 0, // Translation in x, y, z directions
  ROTATION = 1,    // Rotation in the form of euler angles (phi, theta, psi)
  SCALE = 2        // Scaling in x, y, z directions
};

/// MooseEnum for transformation types, matching the TransformationType enum values
static const MooseEnum transformation_type_enum{"TRANSLATION=0 ROTATION=1 SCALE=2"};

/**
 * Class for managing transformations in CSG objects
 */
class CSGTransformationHelper
{
public:
  /**
   * Default constructor
   */
  CSGTransformationHelper() = default;

  /**
   * Get the list of transformations
   * @return The list of transformations
   */
  const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
  getTransformations() const
  {
    return _transformations;
  }

  /**
   * Check if the transformation value is valid for the given type
   * @param type The type of transformation
   * @param values The values for the transformation
   * @return True if the values are valid for the type
   */
  static bool isValidTransformationValue(TransformationType type,
                                         const std::tuple<Real, Real, Real> & values);

  /**
   * Get the string representation of the transformation type.
   * @param type The transformation type
   * @return String name of the transformation type
   */
  static std::string getTransformationTypeString(TransformationType type);

  /**
   * Get the transformations of this object with string representations for types.
   * @return Vector of transformation pairs with string representations for types
   */
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
  getTransformationsAsStrings() const;

protected:
  /**
   * Add a transformation to the list of transformations
   * @param type The type of transformation
   * @param values The values for the transformation
   */
  void addTransformation(TransformationType type, const std::tuple<Real, Real, Real> & values);

  /// List of transformations applied to this object
  std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> _transformations;

  // CSGBase needs to be a friend to access addTransformation
  friend class CSGBase;
};

} // namespace CSG
