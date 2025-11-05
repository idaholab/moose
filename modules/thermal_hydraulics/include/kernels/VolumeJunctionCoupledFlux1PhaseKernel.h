//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class ADVolumeJunction1PhaseUserObject;
class ADNumericalFlux3EqnBase;
class SinglePhaseFluidProperties;

/**
 * Applies a flux to the volume junction for VolumeJunctionCoupledFlux1Phase.
 */
class VolumeJunctionCoupledFlux1PhaseKernel : public ADKernel
{
public:
  static InputParameters validParams();

  VolumeJunctionCoupledFlux1PhaseKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Index within local system of the equation upon which this object acts
  const unsigned int _equation_index;

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
};
