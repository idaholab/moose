#ifndef LSHPLASTICMATERIALRZ_H
#define LSHPLASTICMATERIALRZ_H

#include "SolidMechanicsMaterialRZ.h"

//Forward Declarations
class LSHPlasticMaterialRZ;

template<>
InputParameters validParams<LSHPlasticMaterialRZ>();

/**
 * Plastic material
 */
class LSHPlasticMaterialRZ : public SolidMechanicsMaterialRZ
{
public:
  LSHPlasticMaterialRZ(std::string name,
                       InputParameters parameters);

protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain);

  Real _yield_stress;
  Real _hardening_constant;
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

#endif //CLSHPLASTICMATERIALRZ_H
