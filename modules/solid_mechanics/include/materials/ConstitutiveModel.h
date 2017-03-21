/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONSTITUTIVEMODEL_H
#define CONSTITUTIVEMODEL_H

#include "Material.h"

#include "SymmElasticityTensor.h"
#include "SymmTensor.h"

/**
 */

class ConstitutiveModel : public Material
{
public:
  ConstitutiveModel(const InputParameters & parameters);

  virtual ~ConstitutiveModel() {}

  virtual void initStatefulProperties(unsigned int n_points);

  virtual void computeStress(const Elem & /*current_elem*/,
                             unsigned /*qp*/,
                             const SymmElasticityTensor & elasticityTensor,
                             const SymmTensor & stress_old,
                             SymmTensor & strain_increment,
                             SymmTensor & stress_new);

  virtual bool modifyStrainIncrement(const Elem & /*elem*/,
                                     unsigned qp,
                                     SymmTensor & strain_increment,
                                     SymmTensor & d_strain_dT)
  {
    return applyThermalStrain(qp, strain_increment, d_strain_dT);
  }
  virtual bool updateElasticityTensor(unsigned /*qp*/, SymmElasticityTensor & /*elasticityTensor*/)
  {
    return false;
  }

  virtual bool
  applyThermalStrain(unsigned qp, SymmTensor & strain_increment, SymmTensor & d_strain_dT);

protected:
  const bool _has_temp;
  const VariableValue & _temperature;
  const VariableValue & _temperature_old;
  const Real _alpha;
  Function * _alpha_function;
  bool _has_stress_free_temp;
  Real _stress_free_temp;
  bool _mean_alpha_function;
  Real _ref_temp;

  ///@{ Restartable data to check for the zeroth and first time steps
  bool & _step_zero_cm;
  bool & _step_one_cm;
  ///@}

private:
  using Material::computeProperties;
  using Material::_qp;
};

template <>
InputParameters validParams<ConstitutiveModel>();

#endif // CONSTITUTIVEMODEL_H
