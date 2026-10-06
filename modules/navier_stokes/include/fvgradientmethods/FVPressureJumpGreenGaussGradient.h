//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "FVGradientMethod.h"

class LinearFVGradientReader;
class PorousRhieChowMassFlux;

/**
 * Green-Gauss pressure gradient that removes prescribed jumps before interpolating across baffles.
 */
class FVPressureJumpGreenGaussGradient : public FVGradientMethod
{
public:
  static InputParameters validParams();
  FVPressureJumpGreenGaussGradient(const InputParameters & params);

  /// Link this method to the porous Rhie-Chow object that supplies pressure jumps.
  void linkFlowSystem(PorousRhieChowMassFlux & rc,
                      const LinearFVGradientReader & pressure_gradient);

private:
  void computeGradientWithoutLimiter(
      SystemBase & system,
      GradientContainer & gradient,
      const std::unordered_set<unsigned int> & variable_numbers) const override;

  /// Porous Rhie-Chow object supplying baffle locations and signed pressure jumps after linkage.
  const PorousRhieChowMassFlux * _rhie_chow = nullptr;

  /// Pressure system to which this method is linked.
  const SystemBase * _pressure_system = nullptr;

  /// Pressure variable number within the linked system.
  unsigned int _pressure_variable_number = libMesh::invalid_uint;
};
