#include "ExampleImplicitEuler.h"

#include "Material.h"

template<>
InputParameters validParams<ExampleImplicitEuler>()
{
  InputParameters params = validParams<ImplicitEuler>();
  return params;
}

ExampleImplicitEuler::ExampleImplicitEuler(std::string name,
                                           MooseSystem &sys,
                                           InputParameters parameters)
  :ImplicitEuler(name,sys,parameters),
   _time_coefficient(getMaterialProperty<Real>("time_coefficient"))
{}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return _time_coefficient[_qp]*ImplicitEuler::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return _time_coefficient[_qp]*ImplicitEuler::computeQpJacobian();
}
