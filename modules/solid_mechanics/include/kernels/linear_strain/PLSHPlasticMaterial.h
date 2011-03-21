#ifndef PLSHPLASTICMATERIAL_H
#define PLSHPLASTICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class PLSHPlasticMaterial;

template<>
InputParameters validParams<PLSHPlasticMaterial>();

/**
 * Plastic material
 */
class PLSHPlasticMaterial : public LinearIsotropicMaterial
{
public:
  PLSHPlasticMaterial(std::string name,
                  InputParameters parameters);
  
protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain);
  virtual void computeStress(const RealVectorValue & x, const RealVectorValue & y, const RealVectorValue & z, RealTensorValue & stress);
  

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

  MaterialProperty<ColumnMajorMatrix> & _total_strain;
  MaterialProperty<ColumnMajorMatrix> & _total_strain_old;
  MaterialProperty<RealTensorValue> & _stress;  
  MaterialProperty<RealTensorValue> & _stress_old;  
  MaterialProperty<Real> & _hardening_variable;  
  MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<ColumnMajorMatrix> & _plastic_strain;
  MaterialProperty<ColumnMajorMatrix> & _plastic_strain_old;
  
  ColumnMajorMatrix _identity;  
};

#endif //PLSHPLASTICMATERIAL_H
