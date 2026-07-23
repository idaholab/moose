//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

/**
 * Accumulates the cohesive work of separation Gc = integral of traction . d(displacement jump)
 * over the loading history using the trapezoidal rule. It reads the standard cohesive-zone
 * properties (interface_traction and interface_displacement_jump), so it works with any local
 * traction-separation law, in both the AD and non-AD formulations.
 */
template <bool is_ad>
class CZMGcIntegralTempl : public InterfaceMaterial
{
public:
  static InputParameters validParams();
  CZMGcIntegralTempl(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// Base name of the material system
  const std::string _base_name;

  /// The current and old interface traction (old is always non-AD)
  ///@{
  const GenericMaterialProperty<RealVectorValue, is_ad> & _interface_traction_new;
  const MaterialProperty<RealVectorValue> & _interface_traction_old;
  ///@}

  /// The current and old interface displacement jump (old is always non-AD)
  ///@{
  const GenericMaterialProperty<RealVectorValue, is_ad> & _interface_displacement_jump_new;
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;
  ///@}

  /// The accumulated integral of traction . d(displacement jump), i.e. Gc
  MaterialProperty<Real> & _gc_integral;

  /// The old value of the accumulated integral
  const MaterialProperty<Real> & _gc_integral_old;
};

typedef CZMGcIntegralTempl<false> CZMGcIntegral;
typedef CZMGcIntegralTempl<true> ADCZMGcIntegral;
