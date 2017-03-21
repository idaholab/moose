/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ISOTROPICPLASTICITYSTRESSUPDATE_H
#define ISOTROPICPLASTICITYSTRESSUPDATE_H

#include "RadialReturnStressUpdate.h"

/**
 * This class uses the Discrete material in a radial return isotropic plasticity
 * model.  This class is one of the basic radial return constitutive models;
 * more complex constitutive models combine creep and plasticity.
 *
 * This class inherits from RadialReturnStressUpdate and must be used
 * in conjunction with ComputeReturnMappingStress.  This class calculates
 * an effective trial stress, an effective scalar plastic strain
 * increment, and the derivative of the scalar effective plastic strain increment;
 * these values are passed to the RadialReturnStressUpdate to compute
 * the radial return stress increment.  This isotropic plasticity class also
 * computes the plastic strain as a stateful material property.
 *
 * This class is based on the implicit integration algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University
 * Press, pg. 146 - 149.
 */

class IsotropicPlasticityStressUpdate : public RadialReturnStressUpdate
{
public:
  IsotropicPlasticityStressUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeStressInitialize(Real effectiveTrialStress) override;
  virtual Real computeResidual(Real effectiveTrialStress, Real scalar) override;
  virtual Real computeDerivative(Real effectiveTrialStress, Real scalar) override;
  virtual void iterationFinalize(Real scalar) override;
  virtual void computeStressFinalize(const RankTwoTensor & plasticStrainIncrement) override;

  virtual void computeYieldStress();
  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  Function * _yield_stress_function;
  Real _yield_stress;
  const Real _hardening_constant;
  Function * _hardening_function;

  Real _yield_condition;
  Real _hardening_slope;
  Real _shear_modulus;

  MaterialProperty<RankTwoTensor> & _plastic_strain;
  MaterialProperty<RankTwoTensor> & _plastic_strain_old;
  MaterialProperty<Real> & _scalar_plastic_strain;
  MaterialProperty<Real> * _scalar_plastic_strain_old;

  MaterialProperty<Real> & _hardening_variable;
  MaterialProperty<Real> & _hardening_variable_old;
  const VariableValue & _temperature;
};

template <>
InputParameters validParams<IsotropicPlasticityStressUpdate>();

#endif // ISOTROPICPLASTICITYSTRESSUPDATE_H
