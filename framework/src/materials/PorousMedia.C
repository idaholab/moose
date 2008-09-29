#include "PorousMedia.h"

template<>
Parameters valid_params<PorousMedia>()
{
  Parameters params;
  params.set<Real>("thermal_conductivity")=1.0;
  params.set<Real>("thermal_conductivity_fluid")=0.3707; //[W/(m.K)]
  params.set<Real>("thermal_conductivity_solid")=80.0;   //[W/(m.K)]
  params.set<Real>("specific_heat")=1.0;
  params.set<Real>("specific_heat_fluid")=5195;  //[J/(kg.K)]
  params.set<Real>("specific_heat_solid")=1725;
  params.set<Real>("density")=1.0;
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
  params.set<Real>("gas_constant")=1.0;
  params.set<Real>("porosity")=0.5;
  
  return params;
}

void
PorousMedia::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    Real  temp_diff = _temp[qp]-_my_temp0;

    _thermal_conductivity[qp]          = _my_thermal_conductivity;
    _thermal_conductivity_fluid[qp]    = _my_thermal_conductivity_fluid;
    _thermal_conductivity_solid[qp]    = _my_thermal_conductivity_solid;
    _specific_heat[qp]                 = _my_specific_heat;
    _specific_heat_fluid[qp]           = _my_specific_heat_fluid;
    _specific_heat_solid[qp]           = _my_specific_heat_solid;
    _heat_xfer_coefficient[qp]         = _my_heat_xfer_coefficient;
    _fluid_resistance_coefficient[qp]  = _my_fluid_resistance_coefficient;
    _gas_constant[qp]                  = _my_gas_constant;
    _porosity[qp]                      = _my_porosity;
    _neutron_diffusion_coefficient[qp] = _my_d0+_my_d1*temp_diff;
    _neutron_absorption_xs[qp]         = _my_siga0+_my_siga1*temp_diff;
    _neutron_fission_xs[qp]            = _my_sigf0+_my_sigf1*temp_diff;
    _neutron_per_power[qp]             = _my_neutron_per_power;
    _neutron_per_fission[qp]           = _my_neutron_per_fission;
    _neutron_velocity[qp]              = _my_neutron_velocity;
  }
}
