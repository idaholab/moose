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

class ADVolumeJunction1PhaseUserObject;
class ADNumericalFlux3EqnBase;
class SinglePhaseFluidProperties;

/**
 * Computes a mass or energy flux for VolumeJunctionCoupledFlux1Phase.
 */
class VolumeJunctionCoupledFlux1PhasePostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  VolumeJunctionCoupledFlux1PhasePostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override;

protected:
  /// Equation for which to get flux
  const MooseEnum _equation;

  /// Pressure
  const PostprocessorValue & _p;
  /// Temperature
  const PostprocessorValue & _T;

  /// Coupled area between junction and other application
  const Real _A_coupled;

  /// Normal vector from the junction to the other application
  const RealVectorValue & _normal_from_junction;
  /// Normal vector to the junction from the other application
  const RealVectorValue _normal_to_junction;

  /// Volume junction user object
  const ADVolumeJunction1PhaseUserObject & _volume_junction_uo;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux_uo;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;

  /// Value of this PP
  Real _value;
};
