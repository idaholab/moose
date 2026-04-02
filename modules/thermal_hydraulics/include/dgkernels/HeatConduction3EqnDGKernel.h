//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

/**
 * Adds heat conduction for the single-phase flow model.
 */
class HeatConduction3EqnDGKernel : public ADDGKernel
{
public:
  static InputParameters validParams();

  HeatConduction3EqnDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Thermal conductivity for current element
  const ADMaterialProperty<Real> & _k_elem;
  /// Thermal conductivity for neighbor element
  const ADMaterialProperty<Real> & _k_neig;

  /// Temperature for current element
  const ADMaterialProperty<Real> & _T_elem;
  /// Temperature for neighbor element
  const ADMaterialProperty<Real> & _T_neig;

  /// Cross-sectional area
  const ADVariableValue & _A;
  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;
};
