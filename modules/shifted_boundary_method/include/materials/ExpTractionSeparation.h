//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCZMComputeLocalTractionTotalBase.h"
#include "BoundaryShortestDistanceToSurface.h"

/**
 * Implementation of the non-stateful exponential traction separation law
 **/
class ExpTractionSeparation : public ADCZMComputeLocalTractionTotalBase
{
public:
  static InputParameters validParams();
  ExpTractionSeparation(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// method computing the total traction and its derivatives
  void computeInterfaceTraction() override;

  /// Fracture energy Gc
  const Real _Gc;

  /// Softening length
  const Real _delta0;

  /// Tangential weighting
  const Real _beta;

  /// eps to prevent division by zero
  const Real _eps;

  /// Irreversible damage flag
  const bool _irreversible_damage;

  /// Displacement jump in interface coordinates
  const ADMaterialProperty<RealVectorValue> & _interface_displacement_jump;

  /// Effective displacement jump in interface coordinates
  ADMaterialProperty<RealVectorValue> & _interface_effective_displacement_jump;

  /// Effective displacement jump scalar
  ADMaterialProperty<Real> & _effective_displacement_jump_scalar_max;

  /// Old effective displacement jump scalar
  const MaterialProperty<Real> & _effective_displacement_jump_scalar_max_old;

  /// Damage variable
  ADMaterialProperty<Real> & _damage;
};
