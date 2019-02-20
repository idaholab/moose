#include "OneDAreaTimesConstantBC.h"
#include "VolumeFractionMapper.h"

registerMooseObject("THMApp", OneDAreaTimesConstantBC);

template <>
InputParameters
validParams<OneDAreaTimesConstantBC>()
{
  InputParameters params = validParams<OneDNodalBC>();
  params.addRequiredParam<Real>("value", "The constant value used.");
  params.addRequiredCoupledVar("A", "Area");
  params.declareControllable("value");
  return params;
}

OneDAreaTimesConstantBC::OneDAreaTimesConstantBC(const InputParameters & parameters)
  : OneDNodalBC(parameters), _value(getParam<Real>("value")), _area(coupledValue("A"))
{
}

Real
OneDAreaTimesConstantBC::computeQpResidual()
{
  return _u[_qp] - _area[_qp] * _value;
}
