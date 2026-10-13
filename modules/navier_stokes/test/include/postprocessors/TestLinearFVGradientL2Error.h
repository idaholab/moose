//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralPostprocessor.h"

class LinearFVGradientReader;

/** Computes the L2 error of a named linear FV gradient against a constant exact gradient. */
class TestLinearFVGradientL2Error : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestLinearFVGradientL2Error(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  PostprocessorValue getValue() const override;

private:
  /// Reader for the selected variable and gradient method.
  const LinearFVGradientReader * _gradient;

  /// Constant manufactured gradient.
  const RealVectorValue _exact_gradient;

  /// Domain-integrated squared gradient error, reduced and square-rooted in finalize().
  Real _error;
};
