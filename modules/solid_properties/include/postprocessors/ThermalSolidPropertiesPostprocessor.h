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

class ThermalSolidProperties;

/**
 * Computes a property from a ThermalSolidProperties object.
 */
class ThermalSolidPropertiesPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ThermalSolidPropertiesPostprocessor(const InputParameters & parameters);

  /// Property to query
  enum class Property
  {
    DENSITY,
    SPECIFIC_HEAT,
    THERMAL_CONDUCTIVITY
  };

  virtual void initialize() override;
  virtual void execute() override;
  virtual PostprocessorValue getValue() override;

protected:
  /// Solid properties
  const ThermalSolidProperties & _solid_properties;

  /// Temperature
  const PostprocessorValue & _T;

  /// Property to query
  const Property _property;
};
