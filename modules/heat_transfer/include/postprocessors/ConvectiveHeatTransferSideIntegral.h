//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Computes the total convective heat transfer across a boundary
 */
template <bool is_ad>
class ConvectiveHeatTransferSideIntegralTempl : public SideIntegralPostprocessor
{
public:
  ConvectiveHeatTransferSideIntegralTempl(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeQpIntegral() override;

  /// wall temperature variable
  const VariableValue & _T_wall;

  /// fluid temperature variable
  const VariableValue * const _T_fluid;

  /// fluid temperature variable
  const GenericMaterialProperty<Real, is_ad> * const _T_fluid_mat;

  /// the heat transfer coefficient variable
  const VariableValue * const _hw;

  /// the heat transfer coefficient material, either variable or matprop need to be provided
  const GenericMaterialProperty<Real, is_ad> * const _hw_mat;
};

typedef ConvectiveHeatTransferSideIntegralTempl<false> ConvectiveHeatTransferSideIntegral;
typedef ConvectiveHeatTransferSideIntegralTempl<true> ADConvectiveHeatTransferSideIntegral;
