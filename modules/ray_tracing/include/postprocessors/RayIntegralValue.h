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

// Local includes
#include "Ray.h"

// Forward declarations
class RayTracingStudy;

/**
 * Obtains the integrated value accumulated into a Ray from an IntegralRayKernel-derived class
 */
class RayIntegralValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RayIntegralValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}

  virtual Real getValue() override;

private:
  /// The Ray data index where the integral value resides
  unsigned int _ray_data_index;
  /// The ID of the Ray that we seek the value from
  RayID _ray_id;
  /// The RayTracingStudy to get the value from
  const RayTracingStudy * _study;
};
