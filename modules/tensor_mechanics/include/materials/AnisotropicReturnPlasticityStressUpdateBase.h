//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralizedRadialReturnStressUpdate.h"

template <bool is_ad>
using GenericGeneralizedRadialReturnStressUpdate =
    typename std::conditional<is_ad,
                              ADGeneralizedRadialReturnStressUpdate,
                              GeneralizedRadialReturnStressUpdate>::type;

/**
 * This class provides baseline functionality for anisotropic (Hill-like) plasticity models based
 * on the stress update material in a generalized (Hill-like) radial return calculations.
 */

template <bool is_ad>
class AnisotropicReturnPlasticityStressUpdateBaseTempl
  : public GenericGeneralizedRadialReturnStressUpdate<is_ad>
{
public:
  static InputParameters validParams();

  AnisotropicReturnPlasticityStressUpdateBaseTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  /**
   * Does the model require the elasticity tensor to be isotropic?
   */
  bool requiresIsotropicTensor() override { return false; }

  /**
   * Calculate the derivative of the strain increment with respect to the updated stress.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual GenericReal<is_ad> computeStressDerivative(const Real /*effective_trial_stress*/,
                                                     const Real /*scalar*/) override
  {
    return 0.0;
  }

  /**
   * Perform any necessary steps to finalize strain increment after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   * @param stress Cauchy stress tensor
   * @param stress_dev Deviatoric part of the Cauchy stress tensor
   * @param delta_gamma Plastic multiplier
   */
  virtual void computeStrainFinalize(GenericRankTwoTensor<is_ad> & /*inelasticStrainIncrement*/,
                                     const GenericRankTwoTensor<is_ad> & /*stress*/,
                                     const GenericDenseVector<is_ad> & /*stress_dev*/,
                                     const GenericReal<is_ad> & /*delta_gamma*/) override;

  /// Plasticity strain tensor material property
  GenericMaterialProperty<RankTwoTensor, is_ad> & _plasticity_strain;
  const MaterialProperty<RankTwoTensor> & _plasticity_strain_old;
};

typedef AnisotropicReturnPlasticityStressUpdateBaseTempl<false>
    AnisotropicReturnPlasticityStressUpdateBase;
typedef AnisotropicReturnPlasticityStressUpdateBaseTempl<true>
    ADAnisotropicReturnPlasticityStressUpdateBase;
