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
  virtual void computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain);
  virtual void computeStress(const ColumnMajorMatrix & strain,
                             RealTensorValue & stress);


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

  MaterialProperty<ColumnMajorMatrix> & _total_strain;
  MaterialProperty<ColumnMajorMatrix> & _total_strain_old;
  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<RealTensorValue> & _stress_old;
  MaterialProperty<RealTensorValue> & _creep_strain;
  MaterialProperty<RealTensorValue> & _creep_strain_old;

  ColumnMajorMatrix _identity;
};

#endif //POWERLAWCREEPMATERIAL_H
