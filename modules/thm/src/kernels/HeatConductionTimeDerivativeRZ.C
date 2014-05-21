#include "HeatConductionTimeDerivativeRZ.h"

template<>
InputParameters validParams<HeatConductionTimeDerivativeRZ>()
{
  InputParameters params = validParams<HeatConductionTimeDerivative>();
  params.addRequiredParam<Real>("axial_offset", "");

  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const std::string & name, InputParameters parameters) :
    HeatConductionTimeDerivative(name, parameters),
    _axial_offset(getParam<Real>("axial_offset"))
{
}

HeatConductionTimeDerivativeRZ::~HeatConductionTimeDerivativeRZ()
{
}

Real
HeatConductionTimeDerivativeRZ::computeQpResidual()
{
  Real r = _q_point[_qp](1) + _axial_offset;
  return 2 * M_PI * r * HeatConductionTimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivativeRZ::computeQpJacobian()
{
  Real r = _q_point[_qp](1) + _axial_offset;
  return 2 * M_PI * r * HeatConductionTimeDerivative::computeQpJacobian();
}
