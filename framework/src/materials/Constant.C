#include "Constant.h"

template<>
Parameters valid_params<Constant>()
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
  
  return params;
}

void
Constant::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    _density[qp] = _my_density;
    _thermal_conductivity[qp] = _my_thermal_conductivity;
    _specific_heat[qp] = _my_specific_heat;
    _thermal_expansion[qp] = _my_thermal_expansion;
    _youngs_modulus[qp]  = _my_youngs_modulus;
    _poissons_ratio[qp] = _my_poissons_ratio;
    _neutron_diffusion_coefficient[qp] = _my_neutron_diffusion_coefficient;
    _neutron_absorption_xs[qp] = _my_neutron_absorption_xs;
    _neutron_fission_xs[qp]    = _my_neutron_fission_xs;
    _neutron_per_fission[qp]   = _my_neutron_per_fission;
  }
}
