#include "HeatConductionTimeDerivativeRZ.h"

template<>
InputParameters validParams<HeatConductionTimeDerivativeRZ>()
{
  InputParameters params = validParams<HeatConductionTimeDerivative>();
  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const InputParameters & parameters) :
    HeatConductionTimeDerivative(parameters)
{
}

HeatConductionTimeDerivativeRZ::~HeatConductionTimeDerivativeRZ()
{
}

Real
HeatConductionTimeDerivativeRZ::computeQpResidual()
{
  Real r = _q_point[_qp](1);
  return 2 * M_PI * r * HeatConductionTimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivativeRZ::computeQpJacobian()
{
  Real r = _q_point[_qp](1);
  return 2 * M_PI * r * HeatConductionTimeDerivative::computeQpJacobian();
}
