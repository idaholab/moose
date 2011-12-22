#ifndef CLSHPLASTICMATERIAL_H
#define CLSHPLASTICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class CLSHPlasticMaterial;

template<>
InputParameters validParams<CLSHPlasticMaterial>();

/**
 * Plastic material
 */
class CLSHPlasticMaterial : public LinearIsotropicMaterial
{
public:
  CLSHPlasticMaterial(std::string name,
                  InputParameters parameters);

protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain);
  virtual void computeStress(const SymmTensor & strain,
                             SymmTensor & stress);


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

  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;
  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;
  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<SymmTensor> & _plastic_strain;
  MaterialProperty<SymmTensor> & _plastic_strain_old;

};

#endif //CLSHPLASTICMATERIAL_H
