/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SpecificHeatConductionTimeDerivative.h"

template<>
InputParameters validParams<SpecificHeatConductionTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();

  // Density may be changing with deformation, so we must integrate
  // over current volume by setting the use_displaced_mesh flag.
  params.set<bool>("use_displaced_mesh") = true;

  params.addParam<MaterialPropertyName>("specific_heat", "specific_heat", "Property name of the specific heat material property");
  params.addParam<MaterialPropertyName>("density", "density", "Property name of the density material property");
  return params;
}


SpecificHeatConductionTimeDerivative::SpecificHeatConductionTimeDerivative(const InputParameters & parameters) :
    TimeDerivative(parameters),
    _specific_heat(getMaterialProperty<Real>("specific_heat")),
    _density(getMaterialProperty<Real>("density"))
{
}

Real
SpecificHeatConductionTimeDerivative::computeQpResidual()
{
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpResidual();
}

Real
SpecificHeatConductionTimeDerivative::computeQpJacobian()
{
  return _specific_heat[_qp] * _density[_qp] * TimeDerivative::computeQpJacobian();
}
