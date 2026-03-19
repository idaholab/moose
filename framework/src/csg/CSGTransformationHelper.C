//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGTransformationHelper.h"

namespace CSG
{

void
CSGTransformationHelper::addTransformation(TransformationType type,
                                           const std::tuple<Real, Real, Real> & values)
{
  if (!isValidTransformationValue(type, values))
    mooseError("Invalid transformation values provided for transformation type " +
               getTransformationTypeString(type));
  _transformations.emplace_back(type, values);
}

bool
CSGTransformationHelper::isValidTransformationValue(TransformationType type,
                                                    const std::tuple<Real, Real, Real> & values)
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
      if (std::get<0>(values) == 0.0 || std::get<1>(values) == 0.0 || std::get<2>(values) == 0.0)
        return false;
      return true;

    default:
      mooseError("Unknown transformation type");
  }
}

std::string
CSGTransformationHelper::getTransformationTypeString(TransformationType type)
{
  // Set the enum to the value and convert it to string
  MooseEnum enum_copy = transformation_type_enum;
  enum_copy = static_cast<int>(type);
  return std::string(enum_copy);
}

std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
CSGTransformationHelper::getTransformationsAsStrings() const
{
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>> result;
  for (const auto & transform_pair : _transformations)
    result.emplace_back(getTransformationTypeString(transform_pair.first), transform_pair.second);
  return result;
}

} // namespace CSG
