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
  virtual void initStatefulProperties(unsigned n_points);

  virtual void computeStressInitialize(unsigned qp,
                                       Real effectiveTrialStress,
                                       const SymmElasticityTensor & elasticityTensor);
  virtual Real computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar);
  virtual Real computeDerivative(unsigned qp, Real effectiveTrialStress, Real scalar);
  virtual void iterationFinalize(unsigned qp, Real scalar);
  virtual void computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement);

  const Real _yield_stress;
  const Real _hardening_constant;
  const Real _c_alpha;
  const Real _c_beta;

  Real _yield_condition;
  Real _shear_modulus;
  Real _xphir;
  Real _xphidp;

  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;
  MaterialProperty<SymmTensor> & _plastic_strain;
  MaterialProperty<SymmTensor> & _plastic_strain_old;
};

template <>
InputParameters validParams<CLSHPlasticModel>();

#endif // CLSHPLASTICMODEL_H
