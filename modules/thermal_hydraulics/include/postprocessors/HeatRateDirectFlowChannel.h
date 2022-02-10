//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
