//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://github.com/idaholab/moose/blob/master/LICENSE

#pragma once

#include "GeneralPostprocessor.h"

class SubChannel1PhaseProblem;

/**
 * Reports the cross-sectionally homogenized friction and local-form pressure loss of an assembly.
 */
class SCMFrictionPressureDrop : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  SCMFrictionPressureDrop(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override;

protected:
  const SubChannel1PhaseProblem * const _scm_problem;
  Real _value;
};
