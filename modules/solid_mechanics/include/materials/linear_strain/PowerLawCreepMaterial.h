#ifndef POWERLAWCREEPMATERIAL_H
#define POWERLAWCREEPMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class PowerLawCreepMaterial;

template<>
InputParameters validParams<PowerLawCreepMaterial>();

/**
 * Power-law creep material
 * edot = A(sigma)**n * exp(-Q/(RT))
 */
class PowerLawCreepMaterial : public LinearIsotropicMaterial
{
public:
  PowerLawCreepMaterial(std::string name,
                        InputParameters parameters);

protected:
  /**
   * Will always be passed a full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain);
  virtual void computeStress(const SymmTensor & strain,
                             SymmTensor & stress);


  Real _coefficient;
  Real _exponent;
  Real _activation_energy;
  Real _gas_constant;

  Real _tolerance;
  unsigned int _max_its;
  bool _output_iteration_info;

  bool _has_temp;
  VariableValue & _temp;

  Real _shear_modulus;

  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;
  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;
  MaterialProperty<SymmTensor> & _creep_strain;
  MaterialProperty<SymmTensor> & _creep_strain_old;
};

#endif //POWERLAWCREEPMATERIAL_H
