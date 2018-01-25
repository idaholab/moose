//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CLSHPLASTICMODEL_H
#define CLSHPLASTICMODEL_H

#include "ReturnMappingModel.h"

class CLSHPlasticModel;

template <>
InputParameters validParams<CLSHPlasticModel>();

/**
 * Plastic material
 */
class CLSHPlasticModel : public ReturnMappingModel
{
public:
  CLSHPlasticModel(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeStressInitialize(Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor) override;
  virtual Real computeResidual(const Real effectiveTrialStress, const Real scalar) override;
  virtual Real computeDerivative(const Real effectiveTrialStress, const Real scalar) override;
  virtual void iterationFinalize(Real scalar) override;
  virtual void computeStressFinalize(const SymmTensor & plasticStrainIncrement) override;
  Real computeHardeningValue(const Real scalar);

  const Real _yield_stress;
  const Real _hardening_constant;
  const Real _c_alpha;
  const Real _c_beta;

  Real _yield_condition;
  Real _shear_modulus;
  Real _xphir;
  Real _xphidp;

  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<SymmTensor> & _plastic_strain;
  const MaterialProperty<SymmTensor> & _plastic_strain_old;
};

#endif // CLSHPLASTICMODEL_H
