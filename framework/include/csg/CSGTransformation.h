//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseEnum.h"

namespace CSG
{

/**
 * Enumeration of transformation types that can be applied to CSG objects.
 */
enum class TransformationType
{
  TRANSLATION = 0, ///< Translation in x, y, z directions
  ROTATION = 1,    ///< Rotation in the form of euler angles (phi, theta, psi)
  SCALE = 2        ///< Scaling in x, y, z directions
};

/// MooseEnum for transformation types, matching the TransformationType enum values
static const MooseEnum transformation_type_enum{"TRANSLATION=0 ROTATION=1 SCALE=2"};

/**
 * Check if the transformation values are valid for the given transformation type.
 * All transformation types require exactly 3 values.
 * @param type The transformation type
 * @param values Vector of transformation values (must be size 3)
 * @return True if the values are valid for the transformation type
 * @throws If the values vector is not exactly size 3
 */
bool isValidTransformationValue(TransformationType type, const std::vector<Real> & value);

/**
 * Get the string representation of the transformation type.
 * @param type The transformation type
 * @return String name of the transformation type
 */
const std::string getTransformationTypeString(TransformationType type);

} // namespace CSG