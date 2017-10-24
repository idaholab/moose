/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CLSHPLASTICMODEL_H
#define CLSHPLASTICMODEL_H

#include "ReturnMappingModel.h"

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

template <>
InputParameters validParams<CLSHPlasticModel>();

#endif // CLSHPLASTICMODEL_H
