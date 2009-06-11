#include "Material.h"

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

//Forward Declarations
class ExampleMaterial;

template<>
Parameters valid_params<ExampleMaterial>();

/**
 * Example material class that defines a few properties.
 */
class ExampleMaterial : public Material
{
public:
  ExampleMaterial(std::string name,
                  Parameters parameters,
                  unsigned int block_id,
                  std::vector<std::string> coupled_to,
                  std::vector<std::string> coupled_as);

protected:
  virtual void computeProperties();

private:
  Real _input_diffusivity;
  
  std::vector<Real> & _diffusivity;
};

#endif //EXAMPLEMATERIAL_H
