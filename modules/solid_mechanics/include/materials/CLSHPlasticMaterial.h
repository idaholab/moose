#ifndef CLSHPLASTICMATERIAL_H
#define CLSHPLASTICMATERIAL_H

#include "SolidModel.h"

//Forward Declarations
class CLSHPlasticMaterial;

template<>
InputParameters validParams<CLSHPlasticMaterial>();

/**
 * Plastic material
 */
class CLSHPlasticMaterial : public SolidModel
{
public:
  CLSHPlasticMaterial(std::string name,
                      InputParameters parameters);

protected:
  virtual void computeStress();

  Real _yield_stress;
  Real _hardening_constant;
  Real _c_alpha;
  Real _c_beta;
  Real _tolerance;
  unsigned int _max_its;

  bool _print_debug_info;

  /**
   * The shear modulus of the material.
   */
  Real _shear_modulus;
  Real _ebulk3;
  Real _K;

  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<SymmTensor> & _plastic_strain;
  MaterialProperty<SymmTensor> & _plastic_strain_old;

};

#endif //CLSHPLASTICMATERIAL_H
