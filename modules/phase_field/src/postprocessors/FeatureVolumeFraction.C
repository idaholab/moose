//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FeatureVolumeFraction.h"
#include <cmath>

registerMooseObject("PhaseFieldApp", FeatureVolumeFraction);

InputParameters
FeatureVolumeFraction::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum value_type("VOLUME_FRACTION AVRAMI", "VOLUME_FRACTION");
  params.addParam<MooseEnum>(
      "value_type", value_type, "The value to output (VOLUME_FRACTION or AVRAMI value)");
  params.addRequiredParam<PostprocessorName>("mesh_volume",
                                             "Postprocessor from which to get mesh volume");
  params.addRequiredParam<VectorPostprocessorName>("feature_volumes",
                                                   "The feature volume VectorPostprocessorValue.");
  params.addParam<Real>(
      "equil_fraction", -1.0, "Equilibrium volume fraction of 2nd phase for Avrami analysis");
  return params;
}

FeatureVolumeFraction::FeatureVolumeFraction(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value_type(getParam<MooseEnum>("value_type").getEnum<ValueType>()),
    _mesh_volume(getPostprocessorValue("mesh_volume")),
    _feature_volumes(getVectorPostprocessorValue("feature_volumes", "feature_volumes")),
    _equil_fraction(getParam<Real>("equil_fraction")),
    _avrami_value(0)
{
}

void
FeatureVolumeFraction::initialize()
{
}

void
FeatureVolumeFraction::execute()
{
  Real volume = 0.0;

  // sum the values in the vector to get total volume
  for (const auto & feature_volume : _feature_volumes)
    volume += feature_volume;

  mooseAssert(!MooseUtils::absoluteFuzzyEqual(_mesh_volume, 0.0), "Mesh volume is zero");
  _volume_fraction = volume / _mesh_volume;

  _avrami_value = calculateAvramiValue();
}

Real
FeatureVolumeFraction::getValue()
{
  switch (_value_type)
  {
    case ValueType::VOLUME_FRACTION:
      return _volume_fraction;
    case ValueType::AVRAMI:
      return _avrami_value;
    default:
      return 0;
  }
}

Real
FeatureVolumeFraction::calculateAvramiValue()
{
  return std::log(std::log(1.0 / (1.0 - (_volume_fraction / _equil_fraction))));
}
