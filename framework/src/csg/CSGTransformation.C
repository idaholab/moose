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
isValidTransformationValue(TransformationType type, const std::tuple<Real, Real, Real> & value)
{
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
      // Scaling factors should be non-zero
      if (std::get<0>(value) == 0.0 || std::get<1>(value) == 0.0 || std::get<2>(value) == 0.0)
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

std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
convertTransformationsToString(
    const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
        transformations)
{
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>> result;
  result.reserve(transformations.size());
  for (const auto & transform_pair : transformations)
    result.emplace_back(getTransformationTypeString(transform_pair.first), transform_pair.second);
  return result;
}

} // namespace CSG