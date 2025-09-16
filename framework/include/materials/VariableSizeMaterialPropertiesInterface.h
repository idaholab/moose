//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Interface class to return the size of material properties that do not have a fixed size
 */
class VariableSizeMaterialPropertiesInterface
{
public:
  VariableSizeMaterialPropertiesInterface(const InputParameters & /*params*/) {}

  /// Return the size of the variable size vector material property that the material defines
  virtual std::size_t getVectorPropertySize(const MaterialPropertyName & /*prop_name*/) const = 0;
};
