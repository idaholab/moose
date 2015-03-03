/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatConductionTimeDerivative.h"

template<>
InputParameters validParams<HeatConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  // Density may be changing with deformation, so we must integrate over current volume
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}


HeatConductionTimeDerivative::HeatConductionTimeDerivative(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters),
   _specific_heat(getMaterialProperty<Real>("specific_heat")),
   _density(getMaterialProperty<Real>("density"))
{}

Real
HeatConductionTimeDerivative::computeQpResidual()
{
  return _specific_heat[_qp]*_density[_qp]*TimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivative::computeQpJacobian()
{
  return _specific_heat[_qp]*_density[_qp]*TimeDerivative::computeQpJacobian();
}
