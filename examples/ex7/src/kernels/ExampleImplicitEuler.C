#include "ExampleImplicitEuler.h"

#include "Material.h"

ExampleImplicitEuler::ExampleImplicitEuler(std::string name,
                                           InputParameters parameters,
                                           std::string var_name,
                                           std::vector<std::string> coupled_to,
                                           std::vector<std::string> coupled_as)
  :ImplicitEuler(name,parameters,var_name,coupled_to,coupled_as)
{}

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
