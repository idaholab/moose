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
  PlasticMaterial(std::string name,
                  MooseSystem & moose_system,
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

  MooseArray<Real> & _yield_stress;
  MooseArray<Real> & _shear_modulus;

  MooseArray<ColumnMajorMatrix> & _plastic_strain;
  MooseArray<ColumnMajorMatrix> & _plastic_strain_old;
  
  MooseArray<Real> & _accumulated_plastic_strain;
  MooseArray<Real> & _accumulated_plastic_strain_old;

  MooseArray<Real> & _von_mises_stress;

  MooseArray<Real> & _delta_gamma;
};

#endif //PLASTICMATERIAL_H
