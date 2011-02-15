#include "HeatConductionRZ.h"

template<>
InputParameters validParams<HeatConductionRZ>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("Heat conduction for RZ");
  return params;
}

HeatConductionRZ::HeatConductionRZ(const std::string & name, InputParameters parameters)
  :HeatConduction(name, parameters)
  {}

Real
HeatConductionRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * HeatConduction::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * HeatConduction::computeQpJacobian();
}
