//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POWERLAWCREEPMODEL_H
#define POWERLAWCREEPMODEL_H

#include "ReturnMappingModel.h"

class PowerLawCreepModel;

template <>
InputParameters validParams<PowerLawCreepModel>();

class PowerLawCreepModel : public ReturnMappingModel
{
public:
  PowerLawCreepModel(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor) override;
  virtual void computeStressFinalize(const SymmTensor & plasticStrainIncrement) override;

  virtual Real computeResidual(const Real effectiveTrialStress, const Real scalar) override;
  virtual Real computeDerivative(const Real effectiveTrialStress, const Real scalar) override;

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
  const MaterialProperty<SymmTensor> & _creep_strain_old;

private:
};

#endif // POWERLAWCREEPMODEL_H
