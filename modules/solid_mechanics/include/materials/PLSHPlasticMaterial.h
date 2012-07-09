#ifndef PLSHPLASTICMATERIAL_H
#define PLSHPLASTICMATERIAL_H

#include "SolidModel.h"

//Forward Declarations
class PLSHPlasticMaterial;

template<>
InputParameters validParams<PLSHPlasticMaterial>();

/**
 * Plastic material
 */
class PLSHPlasticMaterial : public SolidModel
{
public:
  PLSHPlasticMaterial(std::string name,
                      InputParameters parameters);

protected:
  virtual void initQpStatefulProperties();

  virtual void computeStress();

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

#endif //PLSHPLASTICMATERIAL_H
