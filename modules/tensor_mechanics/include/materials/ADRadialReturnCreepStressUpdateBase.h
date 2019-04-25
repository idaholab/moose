//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADRadialReturnStressUpdate.h"

#define usingRadialReturnCreepStressUpdateBaseMembers                                              \
  usingRadialReturnStressUpdateMembers;                                                            \
  using ADRadialReturnCreepStressUpdateBase<compute_stage>::_creep_strain;                         \
  using ADRadialReturnCreepStressUpdateBase<compute_stage>::_creep_strain_old

// Forward Declarations
template <ComputeStage>
class ADRadialReturnCreepStressUpdateBase;

declareADValidParams(ADRadialReturnCreepStressUpdateBase);

/**
 * This class provides baseline functionallity for creep models based on the stress update material
 * in a radial return isotropic creep calculations.
 */
template <ComputeStage compute_stage>
class ADRadialReturnCreepStressUpdateBase : public ADRadialReturnStressUpdate<compute_stage>
{
public:
  ADRadialReturnCreepStressUpdateBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void computeStressFinalize(const ADRankTwoTensor & plastic_strain_increment) override;

  /// Creep strain material property
  ADMaterialProperty(RankTwoTensor) & _creep_strain;
  const MaterialProperty<RankTwoTensor> & _creep_strain_old;

  usingRadialReturnStressUpdateMembers;
};

