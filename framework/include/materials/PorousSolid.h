#include "Material.h"

#ifndef POROUSSOLID_H
#define POROUSSOLID_H

//Forward Declarations
class PorousSolid;

template<>
Parameters valid_params<PorousSolid>();

/**
 * Simple material used for PebbleBed reactor calculation
 * This is a dummy material with temperature dependence.
 */
class PorousSolid : public Material
{
public:
  PorousSolid(Parameters parameters,
      unsigned int block_id,
      std::vector<std::string> coupled_to,
      std::vector<std::string> coupled_as)
    :Material(parameters,block_id,coupled_to,coupled_as),
    _my_thermal_conductivity(parameters.get<Real>("thermal_conductivity")),
    _my_specific_heat(parameters.get<Real>("specific_heat")),
    _my_density(parameters.get<Real>("density")),
    _my_k0(parameters.get<Real>("k0")),
    _my_k1(parameters.get<Real>("k1")),    
    _my_d0(parameters.get<Real>("d0")),
    _my_d1(parameters.get<Real>("d1")),
    _my_siga0(parameters.get<Real>("siga0")),
    _my_siga1(parameters.get<Real>("siga1")),
    _my_sigf0(parameters.get<Real>("sigf0")),
    _my_sigf1(parameters.get<Real>("sigf1")),
    _my_neutron_per_fission(parameters.get<Real>("neutron_per_fission")),
    _my_neutron_velocity(parameters.get<Real>("neutron_velocity")),
    _my_neutron_per_power(parameters.get<Real>("neutron_per_power")),
    _my_heat_xfer_coefficient(parameters.get<Real>("heat_xfer_coefficient")),
    _my_temp0(parameters.get<Real>("temp0"))
  {}

protected:
  virtual void computeProperties();

private:
  Real _my_thermal_conductivity;
  Real _my_specific_heat;
  Real _my_density;
  Real _my_k0;
  Real _my_k1;
  Real _my_d0;
  Real _my_d1;
  Real _my_siga0;
  Real _my_siga1;
  Real _my_sigf0;
  Real _my_sigf1;
  Real _my_neutron_per_fission;
  Real _my_neutron_velocity;
  Real _my_neutron_per_power;
  Real _my_heat_xfer_coefficient;
  Real _my_temp0;
  
};

#endif //POROUSSOLID_H
