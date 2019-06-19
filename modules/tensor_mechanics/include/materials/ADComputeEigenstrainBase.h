//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

#define usingComputeEigenstrainBaseMembers                                                         \
  usingMaterialMembers;                                                                            \
  using ADComputeEigenstrainBase<compute_stage>::_eigenstrain;                                     \
  using ADComputeEigenstrainBase<compute_stage>::_eigenstrain_name

// Forward Declarations
template <ComputeStage>
class ADComputeEigenstrainBase;

template <typename>
class RankTwoTensorTempl;

typedef RankTwoTensorTempl<Real> RankTwoTensor;
typedef RankTwoTensorTempl<DualReal> DualRankTwoTensor;

declareADValidParams(ADComputeEigenstrainBase);

/**
 * ADComputeEigenstrainBase is the base class for eigenstrain tensors
 */
template <ComputeStage compute_stage>
class ADComputeEigenstrainBase : public ADMaterial<compute_stage>
{
public:
  ADComputeEigenstrainBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  ///Compute the eigenstrain and store in _eigenstrain
  virtual void computeQpEigenstrain() = 0;

  ///Base name prepended to material property name
  const std::string _base_name;

  ///Material property name for the eigenstrain tensor
  std::string _eigenstrain_name;

  ///Stores the current total eigenstrain
  ADMaterialProperty(RankTwoTensor) & _eigenstrain;

  /**
   * Helper function for models that compute the eigenstrain based on a volumetric
   * strain.  This function computes the diagonal components of the eigenstrain tensor
   * as logarithmic strains.
   * @param volumetric_strain The current volumetric strain to be applied
   * @return Current strain in one direction due to volumetric strain, expressed as a logarithmic
   * strain
   */
  ADReal computeVolumetricStrainComponent(const ADReal volumetric_strain) const;

  /// Restartable data to check for the zeroth and first time steps for thermal calculations
  bool & _step_zero;

  usingMaterialMembers;
};

