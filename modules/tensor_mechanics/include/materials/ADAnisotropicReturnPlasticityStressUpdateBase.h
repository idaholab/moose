//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADGeneralizedRadialReturnStressUpdate.h"

/**
 * This class provides baseline functionality for plasticity models based on the stress update
 * material in a generalized (Hill-like) radial return calculations.
 */
class ADAnisotropicReturnPlasticityStressUpdateBase : public ADGeneralizedRadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  ADAnisotropicReturnPlasticityStressUpdateBase(const InputParameters & parameters);

  /**
   * Compute the strain energy rate density for this inelastic model for the current step.
   * @param stress The stress tensor at the end of the step
   * @param strain_rate The strain rate at the end of the step
   * @return The computed strain energy rate density
   */
  virtual Real
  computeStrainEnergyRateDensity(const ADMaterialProperty<RankTwoTensor> & stress,
                                 const ADMaterialProperty<RankTwoTensor> & strain_rate) = 0;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return false; }

  /**
   * Perform any necessary steps to finalize strain increment after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   * @param stress Cauchy stress tensor
   * @param stress_dev Deviatoric part of the Cauchy stress tensor
   * @param delta_gamma Plastic multiplier
   */
  virtual void computeStrainFinalize(ADRankTwoTensor & /*inelasticStrainIncrement*/,
                                     const ADRankTwoTensor & /*stress*/,
                                     const ADDenseVector & /*stress_dev*/,
                                     const ADReal & /*delta_gamma*/) override;

  /// Plasticity strain tensor material property
  ADMaterialProperty<RankTwoTensor> & _plasticity_strain;
  const MaterialProperty<RankTwoTensor> & _plasticity_strain_old;
};
