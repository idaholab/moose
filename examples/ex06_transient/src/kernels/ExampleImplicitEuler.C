#include "ExampleImplicitEuler.h"

#include "Material.h"

template<>
InputParameters validParams<ExampleImplicitEuler>()
{
  InputParameters params = validParams<ImplicitEuler>();
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleImplicitEuler::ExampleImplicitEuler(std::string name,
                                           MooseSystem &sys,
                                           InputParameters parameters)
  :ImplicitEuler(name,sys,parameters),
   // This kernel expects an input parameter named "time_coefficient"
   _time_coefficient(parameters.get<Real>("time_coefficient"))
{}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return _time_coefficient*ImplicitEuler::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return _time_coefficient*ImplicitEuler::computeQpJacobian();
}
