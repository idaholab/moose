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

  /**
   * This is the member reference that will hold the
   * computed values from this material class and can be
   * shared to other MOOSE objects when using
   * one of the declare property methods.
   */
  MaterialProperty<Real> & _diffusivity;
};

#endif //EXAMPLEMATERIAL_H
