//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "ADUtils.h"
#include "THMNames.h"
#include "SinglePhaseFluidProperties.h"

/**
 * Computes several quantities for FlowModel1Phase.
 */
class FlowModel1PhaseFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  FlowModel1PhaseFunctorMaterial(const InputParameters & parameters);

protected:
  /// Adds pressure functor material property
  template <bool is_ad>
  void addPressureFunctorProperty();
  /// Adds temperature functor material property
  template <bool is_ad>
  void addTemperatureFunctorProperty();
  /// Adds velocity functor material property
  template <bool is_ad>
  void addVelocityFunctorProperty();

  /// rho*A
  const Moose::Functor<ADReal> & _rhoA;
  /// rho*u*A
  const Moose::Functor<ADReal> & _rhouA;
  /// rho*E*A
  const Moose::Functor<ADReal> & _rhoEA;
  /// Cross-sectional area
  const Moose::Functor<Real> & _A;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
