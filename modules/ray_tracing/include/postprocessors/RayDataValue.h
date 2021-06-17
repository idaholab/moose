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

#include "Ray.h"

// Forward declarations
class RayTracingStudy;

/**
 * Obtains a Ray data or aux data value from a banked ray.
 */
class RayDataValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RayDataValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  /// The RayTracingStudy
  const RayTracingStudy & _study;

  /// Whether or not to get auxiliary data (if false, get standard data)
  const bool _aux;

  /// The provided Ray name (if any)
  const std::string * const _ray_name;
  /// The provided Ray ID (if any)
  const RayID * const _ray_id;

private:
  /// The index into the Ray's data that the desired RayVariable is at
  unsigned int _ray_data_index;
};
