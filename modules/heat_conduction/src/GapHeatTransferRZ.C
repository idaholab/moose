#include "GapHeatTransferRZ.h"

template<>
InputParameters validParams<GapHeatTransferRZ>()
{
  InputParameters params = validParams<GapHeatTransfer>();
  return params;
}

GapHeatTransferRZ::GapHeatTransferRZ(const std::string & name, InputParameters parameters)
  :GapHeatTransfer(name, parameters)
{
}

Real
GapHeatTransferRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * GapHeatTransfer::computeQpResidual();
}

Real
GapHeatTransferRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * GapHeatTransfer::computeQpJacobian();
}
