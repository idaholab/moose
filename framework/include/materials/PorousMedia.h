#include "Material.h"

#ifndef POROUSMEDIA_H
#define POROUSMEDIA_H

//Forward Declarations
class PorousMedia;

template<>
Parameters valid_params<PorousMedia>();

/**
 * A simple dummy material used for PebbleBed reactor code development.
 * It holds neutronics and heat conduction properties.
 */
class PorousMedia : public Material
{
 public:
  PorousMedia(std::string name,
              Parameters parameters,
              unsigned int block_id,
              std::vector<std::string> coupled_to,
              std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as),
    _my_thermal_conductivity(parameters.get<Real>("thermal_conductivity")),
    _my_thermal_conductivity_fluid(parameters.get<Real>("thermal_conductivity_fluid")),
    _my_thermal_conductivity_solid(parameters.get<Real>("thermal_conductivity_solid")),
    _my_specific_heat(parameters.get<Real>("specific_heat")),
    _my_specific_heat_fluid(parameters.get<Real>("specific_heat_fluid")),
    _my_specific_heat_solid(parameters.get<Real>("specific_heat_solid")),
    _my_neutron_diffusion_coefficient(parameters.get<Real>("neutron_diffusion_coefficient")),
    _my_neutron_absorption_xs(parameters.get<Real>("neutron_absorption_xs")),
    _my_neutron_fission_xs(parameters.get<Real>("neutron_fission_xs")),
    _my_neutron_per_fission(parameters.get<Real>("neutron_per_fission")),
    _my_neutron_velocity(parameters.get<Real>("neutron_velocity")),
    _my_neutron_per_power(parameters.get<Real>("neutron_per_power")),
    _my_heat_xfer_coefficient(parameters.get<Real>("heat_xfer_coefficient")),
    _my_temp0(parameters.get<Real>("temp0")),
    _my_k0(parameters.get<Real>("k0")),
    _my_k1(parameters.get<Real>("k1")),    
    _my_d0(parameters.get<Real>("d0")),
    _my_d1(parameters.get<Real>("d1")),
    _my_siga0(parameters.get<Real>("siga0")),
    _my_siga1(parameters.get<Real>("siga1")),
    _my_sigf0(parameters.get<Real>("sigf0")),
    _my_sigf1(parameters.get<Real>("sigf1")),
    _my_fluid_resistance_coefficient(parameters.get<Real>("fluid_resistance_coefficient")),
    _my_gas_constant(parameters.get<Real>("gas_constant")),
    _my_porosity(parameters.get<Real>("porosity")),
    _my_vessel_cross_section(parameters.get<Real>("vessel_cross_section")),
    _my_pebble_diameter(parameters.get<Real>("pebble_diameter")),
    _my_kta_standard(parameters.get<bool>("kta_standard")),
    _my_non_dim_flag(parameters.get<bool>("non_dim_flag"))
    {
      _gravity(0) = 0.0;
      _gravity(1) = -9.8;
      _gravity(2) = 0.0;
    }


protected:
  virtual void computeProperties();
  
private:
  Real _my_thermal_conductivity;
  Real _my_thermal_conductivity_fluid;
  Real _my_thermal_conductivity_solid;
  Real _my_specific_heat;
  Real _my_specific_heat_fluid;
  Real _my_specific_heat_solid;
  Real _my_neutron_diffusion_coefficient;
  Real _my_neutron_absorption_xs;
  Real _my_neutron_fission_xs;
  Real _my_neutron_per_fission;
  Real _my_neutron_velocity;  
  Real _my_neutron_per_power;
  Real _my_heat_xfer_coefficient;
  Real _my_temp0;
  Real _my_k0;
  Real _my_k1;
  Real _my_d0;
  Real _my_d1;
  Real _my_siga0;
  Real _my_siga1;
  Real _my_sigf0;
  Real _my_sigf1;
  Real _my_fluid_resistance_coefficient;
  Real _my_gas_constant;
  Real _my_porosity;
  Real _my_vessel_cross_section;
  Real _my_pebble_diameter;
  bool _my_kta_standard;
  bool _my_non_dim_flag;
  VectorValue<Real> _gravity;
  VectorValue<Real> _momentum;
  
};

#endif //POROUSMEDIA_H
