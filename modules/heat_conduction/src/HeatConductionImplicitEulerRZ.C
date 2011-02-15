#include "HeatConductionImplicitEulerRZ.h"

template<>
InputParameters validParams<HeatConductionImplicitEulerRZ>()
{
  InputParameters params = validParams<HeatConductionImplicitEuler>();
  return params;
}


HeatConductionImplicitEulerRZ::HeatConductionImplicitEulerRZ(const std::string & name,
                                                             InputParameters parameters)
  :HeatConductionImplicitEuler(name, parameters)
{}

Real
HeatConductionImplicitEulerRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * HeatConductionImplicitEuler::computeQpResidual();
}

Real
HeatConductionImplicitEulerRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * HeatConductionImplicitEuler::computeQpJacobian();
}
