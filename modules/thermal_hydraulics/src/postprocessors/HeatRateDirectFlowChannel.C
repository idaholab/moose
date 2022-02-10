//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateDirectFlowChannel.h"
#include "Function.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateDirectFlowChannel);
registerMooseObject("ThermalHydraulicsApp", ADHeatRateDirectFlowChannel);

template <bool is_ad>
InputParameters
HeatRateDirectFlowChannelTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addRequiredParam<MaterialPropertyName>("q_wall_prop", "Wall heat flux material property");
  params.addRequiredParam<FunctionName>("P_hf", "Heat flux perimeter");

  params.addClassDescription(
      "Computes the heat rate into a flow channel from heat flux material property");

  return params;
}

template <bool is_ad>
HeatRateDirectFlowChannelTempl<is_ad>::HeatRateDirectFlowChannelTempl(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),

    _q_wall(getGenericMaterialProperty<Real, is_ad>("q_wall_prop")),
    _P_hf_fn(getFunction("P_hf"))
{
}

template <bool is_ad>
Real
HeatRateDirectFlowChannelTempl<is_ad>::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_q_wall[_qp]) * _P_hf_fn.value(_t, _q_point[_qp]);
}

template class HeatRateDirectFlowChannelTempl<false>;
template class HeatRateDirectFlowChannelTempl<true>;
