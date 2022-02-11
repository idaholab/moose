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
#include "DerivativeMaterialInterfaceTHM.h"

class SinglePhaseFluidProperties;

/**
 * Computes dynamic viscosity
 */
class DynamicViscosityMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  DynamicViscosityMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Dynamic viscosity name
  const MaterialPropertyName & _mu_name;

  // Dynamic viscosity derivatives
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> * const _dmu_dbeta;
  MaterialProperty<Real> & _dmu_darhoA;
  MaterialProperty<Real> & _dmu_darhouA;
  MaterialProperty<Real> & _dmu_darhoEA;

  /// Specific volume
  const MaterialProperty<Real> & _v;
  const MaterialProperty<Real> * const _dv_dbeta;
  const MaterialProperty<Real> & _dv_darhoA;

  /// Specific internal energy
  const MaterialProperty<Real> & _e;
  const MaterialProperty<Real> & _de_darhoA;
  const MaterialProperty<Real> & _de_darhouA;
  const MaterialProperty<Real> & _de_darhoEA;

  /// Single-phase fluid properties
  const SinglePhaseFluidProperties & _fp_1phase;

public:
  static InputParameters validParams();
};
