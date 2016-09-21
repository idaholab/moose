/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatCapacityConductionTimeDerivative.h"

template<>
InputParameters validParams<HeatCapacityConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>("heat_capacity", "heat_capacity", "Property name of the heat capacity material property");
  return params;
}


HeatCapacityConductionTimeDerivative::HeatCapacityConductionTimeDerivative(const InputParameters & parameters) :
    TimeDerivative(parameters),
    _heat_capacity(getMaterialProperty<Real>("heat_capacity"))
{
}

Real
HeatCapacityConductionTimeDerivative::computeQpResidual()
{
  return _heat_capacity[_qp] * TimeDerivative::computeQpResidual();
}

Real
HeatCapacityConductionTimeDerivative::computeQpJacobian()
{
  return _heat_capacity[_qp] * TimeDerivative::computeQpJacobian();
}
