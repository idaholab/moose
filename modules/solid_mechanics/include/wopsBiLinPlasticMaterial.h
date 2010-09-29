#ifndef WOPSBILINPLASTICMATERIAL_H
#define WOPSBILINPLASTICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class wopsBiLinPlasticMaterial;

template<>
InputParameters validParams<wopsBiLinPlasticMaterial>();

/**
 * Plastic material
 */
class wopsBiLinPlasticMaterial : public LinearIsotropicMaterial
{
public:
  wopsBiLinPlasticMaterial(std::string name,
                  MooseSystem & moose_system,
                  InputParameters parameters);
  
protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain);  

  Real _yield_stress;
  Real _hardening_constant;
  Real _tolerance;
  unsigned int _max_its;

  bool _print_debug_info;

  /**
   * The shear modulus of the material.
   */
  Real _shear_modulus;

  MaterialProperty<ColumnMajorMatrix> & _total_strain;
  MaterialProperty<Real> & _hardening_variable;  
  MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<ColumnMajorMatrix> & _plastic_strain;
  MaterialProperty<ColumnMajorMatrix> & _plastic_strain_old;
  
  ColumnMajorMatrix _identity;  
};

#endif //WOPSBILINPLASTICMATERIAL_H
