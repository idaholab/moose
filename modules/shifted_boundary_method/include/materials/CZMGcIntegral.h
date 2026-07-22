//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeLocalTractionIncrementalBase.h"

/**
 *
 **/
class CZMGcIntegral : public InterfaceMaterial
{
public:
  static InputParameters validParams();
  CZMGcIntegral(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// Base name of the material system
  const std::string _base_name;

  /// the new interface traction value
  const ADMaterialProperty<RealVectorValue> & _interface_traction_new;

  /// the old interface traction value
  const MaterialProperty<RealVectorValue> & _interface_traction_old;

  /// the new effective interface displacement jump
  const ADMaterialProperty<RealVectorValue> & _interface_effective_displacement_jump_new;

  /// The old effective interface displacement jump
  const MaterialProperty<RealVectorValue> & _interface_effective_displacement_jump_old;

  /// The integral of the interface traction multiplied by the effective displacement jump is Gc
  MaterialProperty<Real> & _gc_integral;

  /// The old value of the integral of the interface traction multiplied by the effective displacement jump is Gc
  const MaterialProperty<Real> & _gc_integral_old;
};
