#include "HeatConductionRZ.h"

template<>
InputParameters validParams<HeatConductionRZ>()
{
  InputParameters params = validParams<HeatConductionKernel>();
  return params;
}

HeatConductionRZ::HeatConductionRZ(const InputParameters & parameters) :
    HeatConductionKernel(parameters)
{
}

HeatConductionRZ::~HeatConductionRZ()
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  Real r = _q_point[_qp](1);
  return 2 * M_PI * r * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  Real r = _q_point[_qp](1);
  return 2 * M_PI * r * HeatConductionKernel::computeQpJacobian();
}
