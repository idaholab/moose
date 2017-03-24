#include "OneDEnergyHeatSource.h"
#include "Function.h"

template<>
InputParameters validParams<OneDEnergyHeatSource>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source given by a function");
  params.addRequiredCoupledVar("area", "Cross sectional area");
  return params;
}

OneDEnergyHeatSource::OneDEnergyHeatSource(const InputParameters & parameters) :
    Kernel(parameters),
    _q(getFunction("q")),
    _area(coupledValue("area"))
{
}

Real
OneDEnergyHeatSource::computeQpResidual()
{
  return - _q.value(_t, _q_point[_qp]) * _area[_qp] * _test[_i][_qp];
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
