#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Computes the heat rate into a flow channel from heat flux material property
 */

template <bool is_ad>
class HeatRateDirectFlowChannelTempl : public ElementIntegralPostprocessor
{
public:
  HeatRateDirectFlowChannelTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Wall heat flux
  const GenericMaterialProperty<Real, is_ad> & _q_wall;
  /// Heat flux perimeter
  const Function & _P_hf_fn;

public:
  static InputParameters validParams();
};

typedef HeatRateDirectFlowChannelTempl<false> HeatRateDirectFlowChannel;
typedef HeatRateDirectFlowChannelTempl<true> ADHeatRateDirectFlowChannel;
