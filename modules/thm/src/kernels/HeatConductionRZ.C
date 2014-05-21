#include "HeatConductionRZ.h"

template<>
InputParameters validParams<HeatConductionRZ>()
{
  InputParameters params = validParams<Diffusion>();
  params.addRequiredParam<Real>("axial_offset", "");

  return params;
}

HeatConductionRZ::HeatConductionRZ(const std::string & name, InputParameters parameters) :
    HeatConductionKernel(name, parameters),
    _axial_offset(getParam<Real>("axial_offset"))
{
}

HeatConductionRZ::~HeatConductionRZ()
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  Real r = _q_point[_qp](1) + _axial_offset;
  return 2 * M_PI * r * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  Real r = _q_point[_qp](1) + _axial_offset;
  return 2 * M_PI * r * HeatConductionKernel::computeQpJacobian();
}
