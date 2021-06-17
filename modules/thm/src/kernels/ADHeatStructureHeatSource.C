#include "ADHeatStructureHeatSource.h"

registerMooseObject("THMApp", ADHeatStructureHeatSource);

InputParameters
ADHeatStructureHeatSource::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<Real>("power_fraction", "The fraction of power used");
  params.addRequiredCoupledVar("total_power", "Total reactor power");
  params.addRequiredParam<Real>("num_units", "The number of units");
  params.addRequiredParam<FunctionName>("power_shape_function",
                                        "The name of the function that defines the power shape");
  params.addRequiredParam<PostprocessorName>("power_shape_integral_pp",
                                             "Power shape integral post-processor name");
  params.addParam<Real>("scale", 1.0, "Scaling factor for residual");
  params.declareControllable("power_fraction");
  return params;
}

ADHeatStructureHeatSource::ADHeatStructureHeatSource(const InputParameters & parameters)
  : ADKernel(parameters),
    _power_fraction(getParam<Real>("power_fraction")),
    _total_power(adCoupledScalarValue("total_power")),
    _power_shape_function(getFunction("power_shape_function")),
    _power_shape_integral(getPostprocessorValue("power_shape_integral_pp")),
    _scale(getParam<Real>("scale")),
    _num_units(getParam<Real>("num_units"))
{
}

ADReal
ADHeatStructureHeatSource::computeQpResidual()
{
  const ADReal power = _power_fraction * _total_power[0];
  const ADReal power_density =
      power / (_num_units * _power_shape_integral) * _power_shape_function.value(_t, _q_point[_qp]);
  return -_scale * power_density * _test[_i][_qp];
}
