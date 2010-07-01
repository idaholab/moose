#include "Material.h"

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

//Forward Declarations
class ExampleMaterial;

template<>
InputParameters validParams<ExampleMaterial>();

class ExampleMaterial : public Material
{
public:
  ExampleMaterial(std::string name,
                  MooseSystem & moose_system,
                  InputParameters parameters);

protected:
  virtual void computeProperties();

private:
  Real _input_diffusivity;
  
  MaterialProperty<Real> & _diffusivity;
};

#endif //EXAMPLEMATERIAL_H
