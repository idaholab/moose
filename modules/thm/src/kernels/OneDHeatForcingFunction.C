#include "OneDHeatForcingFunction.h"

template<>
InputParameters validParams<OneDHeatForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("power_fraction", "The fraction of power used");
  params.addRequiredCoupledVar("total_power", "Total reactor power");
  params.addParam<PostprocessorName>("fuel_volume", "The name of the postprocessor that computes fuel volume");
  params.addParam<FunctionName>("power_shape_function", "The name of the function that defines the power shape");

  return params;
}

OneDHeatForcingFunction::OneDHeatForcingFunction(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _power_fraction(getParam<Real>("power_fraction")),
    _total_power(coupledScalarValue("total_power")),
    _fuel_volume(getPostprocessorValue("fuel_volume")),
    _power_shape_function(getFunction("power_shape_function"))
{
}

OneDHeatForcingFunction::~OneDHeatForcingFunction()
{
}

Real
OneDHeatForcingFunction::computeQpResidual()
{
  Real power_density = _power_fraction * _total_power[0] / _fuel_volume;
  Real local_power = power_density * _power_shape_function.value(_t, _q_point[_qp]);
  return -local_power * _test[_i][_qp];
}


