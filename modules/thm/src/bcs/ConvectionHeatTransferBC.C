#include "ConvectionHeatTransferBC.h"

registerMooseObject("THMApp", ConvectionHeatTransferBC);

template <>
InputParameters
validParams<ConvectionHeatTransferBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<Real>("T_ambient", "Ambient Temperature");
  params.addRequiredParam<Real>("htc_ambient", "Heat transfer coefficient with ambient");
  return params;
}

ConvectionHeatTransferBC::ConvectionHeatTransferBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_ambient(getParam<Real>("T_ambient")),
    _htc_ambient(getParam<Real>("htc_ambient"))
{
}

Real
ConvectionHeatTransferBC::computeQpResidual()
{
  return _htc_ambient * (_u[_qp] - _T_ambient) * _test[_i][_qp];
}

Real
ConvectionHeatTransferBC::computeQpJacobian()
{
  return _htc_ambient * _phi[_j][_qp] * _test[_i][_qp];
}
