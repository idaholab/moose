#pragma once

#include "SideIntegralPostprocessor.h"

// Forward Declarations
class ConvectiveHeatTransferSideIntegral;

template <>
InputParameters validParams<ConvectiveHeatTransferSideIntegral>();

/**
 * Computes the total convective heat transfer across a boundary
 */
class ConvectiveHeatTransferSideIntegral : public SideIntegralPostprocessor
{
public:
  ConvectiveHeatTransferSideIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// wall temperature variable
  const VariableValue & _T_wall;

  /// fluid temperature variable
  const VariableValue * _T_fluid;

  /// fluid temperature variable
  const MaterialProperty<Real> * _T_fluid_mat;

  /// the heat transfer coefficient variable
  const VariableValue * _hw;

  /// the heat transfer coefficient material, either variable or matprop need to be provided
  const MaterialProperty<Real> * _hw_mat;
};
