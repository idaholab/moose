//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnStressUpdate.h"

/**
 * This class uses the Discrete material in an isotropic radial return hyperbolic
 * sine viscoplasticity model.
 *
 * This class inherits from RadialReturnStressUpdate and must be used
 * in conjunction with ComputeReturnMappingStress. This uniaxial viscoplasticity
 * class computes the plastic strain as a stateful material property.  The
 * constitutive equation for scalar plastic strain rate used in this model is
 * /f$ \dot{p} = \phi (\sigma_e , r) = \alpha sinh \beta (\sigma_e -r - \sigma_y) f/$
 *
 * This class is based on the implicit integration algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University
 * Press, pg. 162 - 163.
 */
class HyperbolicViscoplasticityStressUpdate : public RadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  HyperbolicViscoplasticityStressUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void computeStressInitialize(const Real & effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual Real computeResidual(const Real & effective_trial_stress, const Real & scalar) override;
  virtual Real computeDerivative(const Real & effective_trial_stress, const Real & scalar) override;
  virtual void iterationFinalize(Real scalar) override;
  virtual void computeStressFinalize(const RankTwoTensor & plasticStrainIncrement) override;
  virtual Real computeHardeningValue(Real scalar);

  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  ///@{ Linear strain hardening parameters
  const Real _yield_stress;
  const Real _hardening_constant;
  ///@}

  ///@{ Viscoplasticity constitutive equation parameters
  const Real _c_alpha;
  const Real _c_beta;
  ///@}

  /// Elastic properties
  Real _yield_condition;

  ///@{ Viscoplasticity terms corresponding to Dunne and Petrinic eqn 5.64
  Real _xphir;
  Real _xphidp;
  ///@}

  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;

  /// plastic strain of this model
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;
};
