//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Exhaustive finite-difference check of averaged mortar nodal-normal coordinate sensitivities.
 */
class NodalNormalSensitivityTest : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NodalNormalSensitivityTest(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

private:
  const bool _collapse_secondary_element;
  const bool _representative_face_only;
  const Real _coordinate_scale;
  const Real _relative_step;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
};
