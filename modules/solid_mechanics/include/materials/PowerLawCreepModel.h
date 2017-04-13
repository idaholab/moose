/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POWERLAWCREEPMODEL_H
#define POWERLAWCREEPMODEL_H

#include "ReturnMappingModel.h"

/**
 */

class PowerLawCreepModel : public ReturnMappingModel
{
public:
  PowerLawCreepModel(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(unsigned qp,
                                       Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);
  virtual void computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement);

  virtual Real computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar);
  virtual Real computeDerivative(unsigned qp, Real effectiveTrialStress, Real scalar);

  const Real _coefficient;
  const Real _n_exponent;
  const Real _m_exponent;
  const Real _activation_energy;
  const Real _gas_constant;
  const Real _start_time;

  Real _shear_modulus;
  Real _exponential;
  Real _expTime;

  MaterialProperty<SymmTensor> & _creep_strain;
  MaterialProperty<SymmTensor> & _creep_strain_old;

private:
};

template <>
InputParameters validParams<PowerLawCreepModel>();

#endif // POWERLAWCREEPMODEL_H
