//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGTransformation.h"
#include "MooseError.h"

namespace CSG
{

bool
isValidTransformationValue(TransformationType type, const std::vector<Real> & value)
{
  // All transformation types require exactly 3 values
  if (value.size() != 3)
    mooseError("Transformation values must be a vector of size 3, but got size ", value.size());

  // Additional validation specific to each transformation type could be added here
  switch (type)
  {
    case TransformationType::TRANSLATION:
      // All translation values are inherently valid
      return true;

    case TransformationType::ROTATION:
      // Rotation uses euler notation; values are angles in degrees (phi, theta, psi)
      // For consistency with TransformGenerator, there are no restrictions on the angles
      // phi: rotation around Z-axis
      // theta: rotation around new X-axis
      // psi: rotation around new Z-axis
      return true;

    case TransformationType::SCALE:
      // Scaling factors should be positive and non-zero
      if (value[0] <= 0.0 || value[1] <= 0.0 || value[2] <= 0.0)
        return false;
      return true;

    default:
      mooseError("Unknown transformation type");
  }
}

const std::string
getTransformationTypeString(TransformationType type)
{
  // Set the enum to the value and convert it to string
  MooseEnum enum_copy = transformation_type_enum;
  enum_copy = static_cast<int>(type);
  return std::string(enum_copy);
}
} // namespace CSG