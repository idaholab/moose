//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Eigenvalues.h"

/**
 * Reports the eigenvalues of a modal analysis together with the angular frequency, frequency, and
 * period of each mode, taking the real part of each eigenvalue as the square of the angular
 * frequency.
 */
class NaturalFrequencies : public Eigenvalues
{
public:
  static InputParameters validParams();

  NaturalFrequencies(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  /// Relative tolerance, scaled by the largest absolute real part of the converged eigenvalues,
  /// below which an eigenvalue is treated as a zero (rigid body) mode
  const Real _zero_eigenvalue_tolerance;

  /// Angular frequency of each mode, in rad/s in a consistent unit system whose time unit is the
  /// second
  VectorPostprocessorValue & _angular_frequency;

  /// Frequency of each mode, in Hz in a consistent unit system whose time unit is the second
  VectorPostprocessorValue & _frequency;

  /// Period of each mode, in s in a consistent unit system whose time unit is the second
  VectorPostprocessorValue & _period;
};
