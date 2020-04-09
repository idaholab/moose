#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Computes the heat rate into a flow channel from heat flux material property
 */
class HeatRateDirectFlowChannel : public ElementIntegralPostprocessor
{
public:
  HeatRateDirectFlowChannel(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Wall heat flux
  const MaterialProperty<Real> & _q_wall;
  /// Heat flux perimeter
  const Function & _P_hf_fn;

public:
  static InputParameters validParams();
};
