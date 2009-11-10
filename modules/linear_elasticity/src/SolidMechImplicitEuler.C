#include "SolidMechImplicitEuler.h"

SolidMechImplicitEuler::SolidMechImplicitEuler(std::string name,
                         InputParameters parameters,
                         std::string var_name,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
    :SecondDerivativeImplicitEuler(name,parameters,var_name,coupled_to,coupled_as)
  {}

void
SolidMechImplicitEuler::subdomainSetup()
  {
    _density = &_material->getRealProperty("density");
  }

Real
SolidMechImplicitEuler::computeQpResidual()
  {
    return (*_density)[_qp]*SecondDerivativeImplicitEuler::computeQpResidual();
  }

Real
SolidMechImplicitEuler::computeQpJacobian()
  {
    return (*_density)[_qp]*SecondDerivativeImplicitEuler::computeQpJacobian();
  }
  
