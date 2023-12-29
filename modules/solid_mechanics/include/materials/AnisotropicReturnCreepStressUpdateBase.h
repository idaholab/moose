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
 * This class provides baseline functionality for anisotropic (Hill-like) plasticity and creep
 * models based on the stress update material in a generalized radial return framework.
 */
template <bool is_ad>
class AnisotropicReturnCreepStressUpdateBaseTempl
  : public GenericGeneralizedRadialReturnStressUpdate<is_ad>
{
public:
  static InputParameters validParams();

  AnisotropicReturnCreepStressUpdateBaseTempl(const InputParameters & parameters);

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
   */
  virtual void computeStrainFinalize(GenericRankTwoTensor<is_ad> & /*inelasticStrainIncrement*/,
                                     const GenericRankTwoTensor<is_ad> & /*stress*/,
                                     const GenericDenseVector<is_ad> & /*stress_dev*/,
                                     const GenericReal<is_ad> & /*delta_gamma*/) override;

  /// Creep strain tensor material property
  GenericMaterialProperty<RankTwoTensor, is_ad> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
};

typedef AnisotropicReturnCreepStressUpdateBaseTempl<false> AnisotropicReturnCreepStressUpdateBase;
typedef AnisotropicReturnCreepStressUpdateBaseTempl<true> ADAnisotropicReturnCreepStressUpdateBase;
