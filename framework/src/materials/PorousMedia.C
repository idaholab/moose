#include "PorousMedia.h"

template<>
Parameters valid_params<PorousMedia>()
{
  Parameters params;
  params.set<Real>("thermal_conductivity")=1.0;
  params.set<Real>("thermal_expansion")=1.0;
  params.set<Real>("specific_heat")=1.0;
  params.set<Real>("density")=1.0;
  params.set<Real>("youngs_modulus")=1.0;
  params.set<Real>("poissons_ratio")=1.0;
  params.set<Real>("neutron_diffusion_coefficient")=1.0;
  params.set<Real>("neutron_absorption_xs")=1.0;
  params.set<Real>("neutron_fission_xs")=1.0;
  params.set<Real>("neutron_per_fission")=1.0;
  params.set<Real>("neutron_velocity")=1.0;
  params.set<Real>("neutron_per_power")=1.0;
  params.set<Real>("heat_xfer_coefficient")=1.0;
  params.set<Real>("temp0")=1.0;
  params.set<Real>("k0")=1.0;
  params.set<Real>("k1")=0.0;
  params.set<Real>("d0")=1.0;
  params.set<Real>("d1")=0.0;
  params.set<Real>("siga0")=1.0;
  params.set<Real>("siga1")=0.0;
  params.set<Real>("sigf0")=1.0;
  params.set<Real>("sigf1")=0.0;
  params.set<Real>("fluid_resistance_coefficient")=1.0;
  params.set<Real>("fluid_conductivity")=1.0;
  params.set<Real>("fluid_specific_heat")=1.0;
  params.set<Real>("gas_constant")=1.0;
  params.set<Real>("gravity")=1.0;
  params.set<Real>("porosity")=0.0;
  
  return params;
}

void
PorousMedia::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    Real  temp_diff = _temp[qp]-_my_temp0;
    
    _thermal_conductivity[qp]          = _my_k0+_my_k1*temp_diff;
    _neutron_diffusion_coefficient[qp] = _my_d0+_my_d1*temp_diff;
    _neutron_absorption_xs[qp]         = _my_siga0+_my_siga1*temp_diff;
    _neutron_fission_xs[qp]            = _my_sigf0+_my_sigf1*temp_diff;
    _neutron_per_power[qp]             = _my_neutron_per_power;
    _heat_xfer_coefficient[qp]         = _my_heat_xfer_coefficient;
    _temp0[qp]                         = _my_temp0;
    _density[qp]                       = _my_density;
    _thermal_conductivity[qp]          = _my_thermal_conductivity;
    _specific_heat[qp]                 = _my_specific_heat;
    _youngs_modulus[qp]                = _my_youngs_modulus;
    _poissons_ratio[qp]                = _my_poissons_ratio;
    _neutron_per_fission[qp]           = _my_neutron_per_fission;
    _neutron_velocity[qp]              = _my_neutron_velocity;
    _fluid_resistance_coefficient[qp]  = _my_fluid_resistance_coefficient;
    _fluid_conductivity[qp]            = _my_fluid_conductivity;
    _fluid_specific_heat[qp]           = _my_fluid_specific_heat;
    _gas_constant[qp]                  = _my_gas_constant;
    _gravity[qp]                       = _my_gravity;
    _porosity[qp]                      = _my_porosity;
  }
}
