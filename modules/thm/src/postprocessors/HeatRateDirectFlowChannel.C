#include "HeatRateDirectFlowChannel.h"
#include "Function.h"

registerMooseObject("THMApp", HeatRateDirectFlowChannel);

InputParameters
HeatRateDirectFlowChannel::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();

  params.addRequiredParam<MaterialPropertyName>("q_wall_prop", "Wall heat flux material property");
  params.addRequiredParam<FunctionName>("P_hf", "Heat flux perimeter");

  params.addClassDescription(
      "Computes the heat rate into a flow channel from heat flux material property");

  return params;
}

HeatRateDirectFlowChannel::HeatRateDirectFlowChannel(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),

    _q_wall(getMaterialProperty<Real>("q_wall_prop")),
    _P_hf_fn(getFunction("P_hf"))
{
}

Real
HeatRateDirectFlowChannel::computeQpIntegral()
{
  return _q_wall[_qp] * _P_hf_fn.value(_t, _q_point[_qp]);
}
