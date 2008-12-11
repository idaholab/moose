#include "Material.h"

#ifndef CONSTANT_H
#define CONSTANT_H

//Forward Declarations
class Constant;

template<>
Parameters valid_params<Constant>();

/**
 * Simple material with constant properties.
 */
class Constant : public Material
{
public:
  Constant(std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as),
    _my_thermal_conductivity(parameters.get<Real>("thermal_conductivity")),
    _my_thermal_expansion(parameters.get<Real>("thermal_expansion")),
    _my_specific_heat(parameters.get<Real>("specific_heat")),
    _my_density(parameters.get<Real>("density")),
    _my_youngs_modulus(parameters.get<Real>("youngs_modulus")),
    _my_poissons_ratio(parameters.get<Real>("poissons_ratio")),
    _my_neutron_diffusion_coefficient(parameters.get<Real>("neutron_diffusion_coefficient")),
    _my_neutron_absorption_xs(parameters.get<Real>("neutron_absorption_xs")),
    _my_neutron_fission_xs(parameters.get<Real>("neutron_fission_xs")),
    _my_neutron_per_fission(parameters.get<Real>("neutron_per_fission")),
    _my_neutron_velocity(parameters.get<Real>("neutron_velocity")),
    _thermal_conductivity(declareRealProperty("thermal_conductivity")),
    _thermal_expansion(declareRealProperty("thermal_expansion")),
    _specific_heat(declareRealProperty("specific_heat")),
    _density(declareRealProperty("density")),
    _youngs_modulus(declareRealProperty("youngs_modulus")),
    _poissons_ratio(declareRealProperty("poissons_ratio")),
    _neutron_diffusion_coefficient(declareRealProperty("neutron_diffusion_coefficient")),
    _neutron_absorption_xs(declareRealProperty("neutron_absorption_xs")),
    _neutron_fission_xs(declareRealProperty("neutron_fission_xs")),
    _neutron_per_fission(declareRealProperty("neutron_per_fission")),
    _neutron_velocity(declareRealProperty("neutron_velocity"))
  {}

protected:
  virtual void computeProperties();

private:
  Real _my_thermal_conductivity;
  Real _my_thermal_expansion;
  Real _my_specific_heat;
  Real _my_density;
  Real _my_youngs_modulus;
  Real _my_poissons_ratio;
  Real _my_neutron_diffusion_coefficient;
  Real _my_neutron_absorption_xs;
  Real _my_neutron_fission_xs;
  Real _my_neutron_per_fission;
  Real _my_neutron_velocity;
  
  std::vector<Real> & _thermal_conductivity;
  std::vector<Real> & _thermal_expansion;
  std::vector<Real> & _specific_heat;
  std::vector<Real> & _density;
  std::vector<Real> & _youngs_modulus;
  std::vector<Real> & _poissons_ratio;
  std::vector<Real> & _neutron_diffusion_coefficient;
  std::vector<Real> & _neutron_absorption_xs;
  std::vector<Real> & _neutron_fission_xs;
  std::vector<Real> & _neutron_per_fission;
  std::vector<Real> & _neutron_velocity;
};

#endif //CONSTANT_H
