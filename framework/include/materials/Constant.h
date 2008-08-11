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
  Constant(Parameters parameters,
      unsigned int block_id,
      std::vector<std::string> coupled_to,
      std::vector<std::string> coupled_as)
    :Material(parameters,block_id,coupled_to,coupled_as),
    _my_thermal_conductivity(parameters.get<Real>("thermal_conductivity")),
    _my_thermal_expansion(parameters.get<Real>("thermal_expansion")),
    _my_specific_heat(parameters.get<Real>("specific_heat")),
    _my_density(parameters.get<Real>("density")),
    _my_youngs_modulus(parameters.get<Real>("youngs_modulus")),
    _my_poissons_ratio(parameters.get<Real>("poissons_ratio"))
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
};

#endif //CONSTANT_H
