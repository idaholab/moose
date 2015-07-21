#include "OneDHeatForcingFunction.h"

template<>
InputParameters validParams<OneDHeatForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("power_fraction", "The fraction of power used");
  params.addRequiredCoupledVar("total_power", "Total reactor power");
  params.addRequiredParam<Real>("volume", "The total heat structure volume");
  params.addParam<FunctionName>("power_shape_function", "The name of the function that defines the power shape");

  return params;
}

OneDHeatForcingFunction::OneDHeatForcingFunction(const InputParameters & parameters) :
    Kernel(parameters),
    _power_fraction(getParam<Real>("power_fraction")),
    _total_power(coupledScalarValue("total_power")),
    _volume(getParam<Real>("volume")),
    _power_shape_function(getFunction("power_shape_function"))
{
}

OneDHeatForcingFunction::~OneDHeatForcingFunction()
{
}

Real
OneDHeatForcingFunction::computeQpResidual()
{
  Real power_density = _power_fraction * _total_power[0] / _volume;
  Real local_power = power_density * _power_shape_function.value(_t, _q_point[_qp]);
  return -local_power * _test[_i][_qp];
}


