//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeEigenstrainBeamBase.h"

/**
 * ComputeThermalExpansionEigenstrainBeamBase is a base class for all models that
 * compute beam eigenstrains due to thermal expansion of a material.
 */
class ComputeThermalExpansionEigenstrainBeamBase : public ComputeEigenstrainBeamBase
{
public:
  static InputParameters validParams();

  ComputeThermalExpansionEigenstrainBeamBase(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain() override;
  /*
   * Compute the total thermal strain relative to the stress-free temperature at
   * the current temperature
   *
   * param thermal_strain    The current total linear thermal strain
   *                         (\delta L / L)
   */
  virtual Real computeThermalStrain() = 0;

  /// Value of temperature at each quadrature point
  const VariableValue & _temperature;

  /// Value of stress free temperature at each quadrature point
  const VariableValue & _stress_free_temperature;

  /// Initial orientation of the beam
  RealGradient _initial_axis;
};
