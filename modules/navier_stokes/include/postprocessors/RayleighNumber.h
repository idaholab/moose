//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

/**
 * This postprocessor computes the Rayleigh number to describe natural circulation
 */
class RayleighNumber : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RayleighNumber(const InputParameters & parameters);

protected:
  void initialize() override {}
  void execute() override {}
  Real getValue() override;

  /// Minimum density
  const PostprocessorValue * const _rho_min;

  /// Maximum density
  const PostprocessorValue * const _rho_max;

  /// Average density
  const PostprocessorValue & _rho_ave;

  /// Thermal expansion coefficient
  const PostprocessorValue * const _beta;

  /// Maximum temperature
  const PostprocessorValue * const _T_hot;

  /// Minimum temperature
  const PostprocessorValue * const _T_cold;

  /// Characteristic length
  const PostprocessorValue & _l;

  /// Average viscosity
  const PostprocessorValue & _mu;

  /// Average thermal conductivity
  const PostprocessorValue & _k;

  /// Average specific thermal capacity
  const PostprocessorValue & _cp;

  /// Magnitude of gravity in the direction of interest
  const Real _gravity;
};
