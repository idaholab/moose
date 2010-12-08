#ifndef PLASTICMATERIAL_H
#define PLASTICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class PlasticMaterial;

template<>
InputParameters validParams<PlasticMaterial>();

/**
 * Plastic material for use in simple applications that don't need material properties.
 */
class PlasticMaterial : public LinearIsotropicMaterial
{
public:
  PlasticMaterial(const std::string & name,
                  InputParameters parameters);
  
protected:
  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(ColumnMajorMatrix & strain);

  /**
   * The point at which plastic strain begins accumulating.
   */
  Real _input_yield_stress;
  
  /**
   * The shear modulus of the material.
   */
  Real _input_shear_modulus;

  MaterialProperty<Real> & _yield_stress;
  MaterialProperty<Real> & _shear_modulus;

  MaterialProperty<ColumnMajorMatrix> & _plastic_strain;
  MaterialProperty<ColumnMajorMatrix> & _plastic_strain_old;
  
  MaterialProperty<Real> & _accumulated_plastic_strain;
  MaterialProperty<Real> & _accumulated_plastic_strain_old;

  MaterialProperty<Real> & _von_mises_stress;

  VariableValue & _delta_gamma;
};

#endif //PLASTICMATERIAL_H
