#include "Material.h"

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

//Forward Declarations
class ExampleMaterial;

template<>
InputParameters validParams<ExampleMaterial>();

/**
 * Example material class that defines a few properties.
 */
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
  Real _input_time_coefficient;
  
  MooseArray<Real> & _diffusivity;
  MooseArray<Real> & _time_coefficient;
};

#endif //EXAMPLEMATERIAL_H
