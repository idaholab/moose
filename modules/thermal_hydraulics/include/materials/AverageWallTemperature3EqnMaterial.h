//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Weighted average of wall temperature between multiple heat sources to
 * preserve total wall heat, for 1-phase flow.
 *
 * See RELAP-7 Theory Manual, pg. 84, Equation (272) {eq:average_wall_temperature_1phase}
 */
class AverageWallTemperature3EqnMaterial : public Material
{
public:
  AverageWallTemperature3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Average wall temperature
  MaterialProperty<Real> & _T_wall;
  /// Number of values to average
  const unsigned int _n_values;
  /// Wall temperature values from the individual sources to average
  std::vector<const VariableValue *> _T_wall_sources;
  /// Wall heat transfer coefficient values from the individual sources to average
  std::vector<const MaterialProperty<Real> *> _Hw_sources;
  /// Heated perimeter values from the individual sources to average
  std::vector<const VariableValue *> _P_hf_sources;
  /// Average wall heat transfer coefficient
  const MaterialProperty<Real> & _Hw_average;
  /// Fluid temperature
  const VariableValue & _T_fluid;
  /// Total heated perimeter
  const VariableValue & _P_hf_total;

public:
  static InputParameters validParams();
};
