//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ISOTROPICPLASTICITY_H
#define ISOTROPICPLASTICITY_H

#include "ReturnMappingModel.h"

class PiecewiseLinear;

class IsotropicPlasticity;

template <>
InputParameters validParams<IsotropicPlasticity>();

class IsotropicPlasticity : public ReturnMappingModel
{
public:
  IsotropicPlasticity(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;

protected:
  virtual void computeYieldStress();
  virtual void computeStressInitialize(Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor) override;
  virtual void computeStressFinalize(const SymmTensor & plasticStrainIncrement) override;

  virtual Real computeResidual(const Real effectiveTrialStress, const Real scalar) override;
  virtual Real computeDerivative(const Real effectiveTrialStress, const Real scalar) override;
  virtual void iterationFinalize(Real scalar) override;

  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  Function * _yield_stress_function;
  Real _yield_stress;
  const Real _hardening_constant;
  PiecewiseLinear * const _hardening_function;

  Real _yield_condition;
  Real _shear_modulus;
  Real _hardening_slope;

  MaterialProperty<SymmTensor> & _plastic_strain;
  const MaterialProperty<SymmTensor> & _plastic_strain_old;

  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
};

#endif // ISOTROPICPLASTICITY_H
