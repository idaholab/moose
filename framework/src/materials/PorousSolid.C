#include "PorousSolid.h"

template<>
Parameters valid_params<PorousSolid>()
{
  Parameters params;
  params.set<Real>("k0")=1.0;
  params.set<Real>("k1")=0.0;
  params.set<Real>("d0")=1.0;
  params.set<Real>("d1")=0.0;
  params.set<Real>("siga0")=1.0;
  params.set<Real>("siga1")=0.0;
  params.set<Real>("sigf0")=1.0;
  params.set<Real>("sigf1")=0.0;
  params.set<Real>("neutron_per_fission")=1.0;
  params.set<Real>("neutron_velocity")=1.0;
  params.set<Real>("temp0")=1.0;
  
  return params;
}

void
PorousSolid::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    Real  temp_diff = _temp[_qp]-_my_temp0;
    
    _density[qp] = _my_density;
    _thermal_conductivity[qp] = _my_k0+_my_k1*temp_diff;
    _neutron_diffusion_coefficient[qp] = _my_d0+_my_d1*temp_diff;
    _neutron_absorption_xs[qp] = _my_siga0+_my_siga1*temp_diff;
    _neutron_fission_xs[qp]    = _my_sigf0+_my_sigf1*temp_diff;
    _neutron_per_fission[qp]   = _my_neutron_per_fission;
    _neutron_velocity[qp]      = _my_neutron_velocity;
  }
}
