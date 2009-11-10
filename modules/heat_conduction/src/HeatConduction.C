#include "HeatConduction.h"
 

template<>
InputParameters valid_params<HeatConduction>()
{
  InputParameters params;
  return params;
}

HeatConduction::HeatConduction(std::string name,
                 InputParameters parameters,
                 std::string var_name,
                 std::vector<std::string> coupled_to,
                 std::vector<std::string> coupled_as)
    :Diffusion(name,parameters,var_name,coupled_to,coupled_as)
  {}

void
HeatConduction::subdomainSetup()
{
  _k = &_material->getRealProperty("thermal_conductivity");
}

Real
HeatConduction::computeQpResidual()
{
  return (*_k)[_qp]*Diffusion::computeQpResidual();
}

Real
HeatConduction::computeQpJacobian()
{
  return (*_k)[_qp]*Diffusion::computeQpJacobian();
}
