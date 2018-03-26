#include "OneDEnergyHeatSource.h"
#include "Function.h"

registerMooseObject("RELAP7App", OneDEnergyHeatSource);

template <>
InputParameters
validParams<OneDEnergyHeatSource>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source given by a function");
  params.addRequiredCoupledVar("A", "Cross sectional area");
  return params;
}

OneDEnergyHeatSource::OneDEnergyHeatSource(const InputParameters & parameters)
  : Kernel(parameters), _q(getFunction("q")), _area(coupledValue("A"))
{
}

Real
OneDEnergyHeatSource::computeQpResidual()
{
  return -_q.value(_t, _q_point[_qp]) * _area[_qp] * _test[_i][_qp];
}

Real
OneDEnergyHeatSource::computeQpJacobian()
{
  return 0.;
}

Real
OneDEnergyHeatSource::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
