#include "HeatConductionImplicitEuler.h"

template<>
InputParameters validParams<HeatConductionImplicitEuler>()
{
  InputParameters params = validParams<ImplicitEuler>();
  return params;
}


HeatConductionImplicitEuler::HeatConductionImplicitEuler(const std::string & name, InputParameters parameters)
  :ImplicitEuler(name, parameters),
   _specific_heat(getMaterialProperty<Real>("specific_heat")),
   _density(getMaterialProperty<Real>("density"))
{}

Real
HeatConductionImplicitEuler::computeQpResidual()
{
  return _specific_heat[_qp]*_density[_qp]*ImplicitEuler::computeQpResidual();
}

Real
HeatConductionImplicitEuler::computeQpJacobian()
{
  return _specific_heat[_qp]*_density[_qp]*ImplicitEuler::computeQpJacobian();
}
