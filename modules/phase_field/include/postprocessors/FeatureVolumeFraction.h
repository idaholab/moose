//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

// Forward Declarations

class FeatureVolumeFraction : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  FeatureVolumeFraction(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  Real calculateAvramiValue();

  enum class ValueType
  {
    VOLUME_FRACTION,
    AVRAMI,
  };

  const ValueType _value_type;
  const PostprocessorValue & _mesh_volume;
  const VectorPostprocessorValue & _feature_volumes;

  Real _volume_fraction;
  Real _equil_fraction;
  Real _avrami_value;
};
