#include "HeatConductionImplicitEuler.h"

HeatConductionImplicitEuler::HeatConductionImplicitEuler(std::string name,
                              Parameters parameters,
                              std::string var_name,
                              std::vector<std::string> coupled_to,
                              std::vector<std::string> coupled_as)
    :ImplicitEuler(name,parameters,var_name,coupled_to,coupled_as)
  {}

void
HeatConductionImplicitEuler::subdomainSetup()
  {
    _specific_heat = &_material->getRealProperty("specific_heat");
    _density = &_material->getRealProperty("density");
  }

Real
HeatConductionImplicitEuler::computeQpResidual()
  {
    return (*_specific_heat)[_qp]*(*_density)[_qp]*ImplicitEuler::computeQpResidual();
  }

Real
HeatConductionImplicitEuler::computeQpJacobian()
  {
    return (*_specific_heat)[_qp]*(*_density)[_qp]*ImplicitEuler::computeQpJacobian();
  }
