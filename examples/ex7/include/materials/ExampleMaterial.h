#include "Material.h"

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

//Forward Declarations
class ExampleMaterial;

template<>
InputParameters valid_params<ExampleMaterial>();

/**
 * Example material class that defines a few properties.
 */
class ExampleMaterial : public Material
{
public:
  ExampleMaterial(std::string name,
                  InputParameters parameters,
                  unsigned int block_id,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as);

protected:
  virtual void computeProperties();

private:
  Real _input_diffusivity;
  Real _input_time_coefficient;
  
  std::vector<Real> & _diffusivity;
  std::vector<Real> & _time_coefficient;
};

#endif //EXAMPLEMATERIAL_H
