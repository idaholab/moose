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
    _my_non_dim_flag(parameters.get<bool>("non_dim_flag")),
    _my_pre0(parameters.get<Real>("pre0")),
    _my_mom0(parameters.get<Real>("mom0")),
    _my_mom1(parameters.get<Real>("mom1")),
    _my_mom2(parameters.get<Real>("mom2")),
    _has_pre(isCoupled("pre")),
    _pre(_has_pre ? coupledVal("pre") : _zero),
    _grad_pre(_has_pre ? coupledGrad("pre") : _grad_zero),
    _has_fluid_temp(isCoupled("fluid_temp")),
    _fluid_temp(_has_fluid_temp ? coupledVal("fluid_temp") : _zero),
    _has_solid_temp(isCoupled("solid_temp")),
    _solid_temp(_has_solid_temp ? coupledVal("solid_temp") : _zero),
    _has_xmom(isCoupled("xmom")),
    _xmom(_has_xmom ? coupledVal("xmom") : _zero),
    _has_ymom(isCoupled("ymom")),
    _ymom(_has_ymom ? coupledVal("ymom") : _zero),
    _has_zmom(isCoupled("zmom")),
    _zmom(_has_zmom ? coupledVal("zmom") : _zero),
    _has_rmom(isCoupled("rmom")),
    _rmom(_has_rmom ? coupledVal("rmom") : _zero),
    _has_thetamom(isCoupled("thetamom")),
    _thetamom(_has_thetamom ? coupledVal("thetamom") : _zero),    
    _thermal_conductivity(declareRealProperty("thermal_conductivity")),
    _thermal_conductivity_fluid(declareRealProperty("thermal_conductivity_fluid")),
    _thermal_conductivity_solid(declareRealProperty("thermal_conductivity_solid")),
    _specific_heat(declareRealProperty("specific_heat")),
    _specific_heat_fluid(declareRealProperty("specific_heat_fluid")),
    _specific_heat_solid(declareRealProperty("specific_heat_solid")),
    _heat_xfer_coefficient(declareRealProperty("heat_xfer_coefficient")),
    _fluid_resistance_coefficient(declareRealProperty("fluid_resistance_coefficient")),
    _gas_constant(declareRealProperty("gas_constant")),
    _porosity(declareRealProperty("porosity")),
    _neutron_diffusion_coefficient(declareRealProperty("neutron_diffusion_coefficient")),
    _neutron_absorption_xs(declareRealProperty("neutron_absorption_xs")),
    _neutron_fission_xs(declareRealProperty("neutron_fission_xs")),
    _neutron_per_power(declareRealProperty("neutron_per_power")),
    _neutron_per_fission(declareRealProperty("neutron_per_fission")),
    _neutron_velocity(declareRealProperty("neutron_velocity"))
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
  Real _my_pre0;
  Real _my_mom0;
  Real _my_mom1;
  Real _my_mom2;
  VectorValue<Real> _gravity;
  VectorValue<Real> _momentum;

  bool _has_pre;
  std::vector<Real> & _pre;
  std::vector<RealGradient> & _grad_pre;

  bool _has_fluid_temp;
  std::vector<Real> & _fluid_temp;
  
  bool _has_solid_temp;
  std::vector<Real> & _solid_temp;

  bool _has_xmom;
  std::vector<Real> & _xmom;
  bool _has_ymom;
  std::vector<Real> & _ymom;
  bool _has_zmom;
  std::vector<Real> & _zmom;
  bool _has_rmom;
  std::vector<Real> & _rmom;
  bool _has_thetamom;
  std::vector<Real> & _thetamom;

  std::vector<Real> & _thermal_conductivity;
  std::vector<Real> & _thermal_conductivity_fluid;
  std::vector<Real> & _thermal_conductivity_solid;
  std::vector<Real> & _specific_heat;
  std::vector<Real> & _specific_heat_fluid;
  std::vector<Real> & _specific_heat_solid;
  std::vector<Real> & _heat_xfer_coefficient;  
  std::vector<Real> & _fluid_resistance_coefficient;
  std::vector<Real> & _gas_constant;  
  std::vector<Real> & _porosity;  
  std::vector<Real> & _neutron_diffusion_coefficient;
  std::vector<Real> & _neutron_absorption_xs;  
  std::vector<Real> & _neutron_fission_xs;  
  std::vector<Real> & _neutron_per_power;  
  std::vector<Real> & _neutron_per_fission;
  std::vector<Real> & _neutron_velocity;
};

#endif //POROUSMEDIA_H
