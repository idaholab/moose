//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class PointInPolyhedronCheckUO;

/**
 * Test-only postprocessor that reports one component of the resolved ray direction of a
 * PointInPolyhedronCheckUO. Used to assert that a user-selected ray_direction is honored and
 * not silently replaced by the PCA auto-selected direction.
 */
class PointInPolyhedronRayDirectionPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  PointInPolyhedronRayDirectionPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

protected:
  /// The point-in-polyhedron user object whose resolved ray direction is reported.
  const PointInPolyhedronCheckUO & _uo;

  /// Component (0=x, 1=y, 2=z) of the ray direction to report.
  const unsigned int _component;
};
