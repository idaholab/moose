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
 * This class provides baseline functionality for anisotropic (Hill-like) plasticity and creep
 * models based on the stress update material in a generalized radial return framework.
 */
class ADAnisotropicReturnCreepStressUpdateBase : public ADGeneralizedRadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  ADAnisotropicReturnCreepStressUpdateBase(const InputParameters & parameters);

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
  virtual void computeStrainFinalize(ADRankTwoTensor & /*inelasticStrainIncrement*/,
                                     const ADRankTwoTensor & /*stress*/,
                                     const ADDenseVector & /*stress_dev*/,
                                     const ADReal & /*delta_gamma*/) override;

  /// Creep strain tensor material property
  ADMaterialProperty<RankTwoTensor> & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;
};
