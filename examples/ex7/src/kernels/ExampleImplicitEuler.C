#include "ExampleImplicitEuler.h"

#include "Material.h"

void
ExampleImplicitEuler::subdomainSetup()
{
  _time_coefficient = &_material->getRealProperty("time_coefficient");
}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return (*_time_coefficient)[_qp]*ImplicitEuler::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return (*_time_coefficient)[_qp]*ImplicitEuler::computeQpJacobian();
}
