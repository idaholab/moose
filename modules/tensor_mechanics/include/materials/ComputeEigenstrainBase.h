//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensorForward.h"

/**
 * ComputeEigenstrainBase is the base class for eigenstrain tensors
 */
template <bool is_ad>
class ComputeEigenstrainBaseTempl : public Material
{
public:
  static InputParameters validParams();

  ComputeEigenstrainBaseTempl(const InputParameters & parameters);

  // We need this to be public for crystal plasticity eigenstrain calculations
  virtual void computeQpProperties() override;

protected:
  virtual void initQpStatefulProperties() override;

  ///Compute the eigenstrain and store in _eigenstrain
  virtual void computeQpEigenstrain() = 0;

  ///Base name prepended to material property name
  const std::string _base_name;

  ///Material property name for the eigenstrain tensor
  std::string _eigenstrain_name;

  ///Stores the current total eigenstrain
  GenericMaterialProperty<RankTwoTensor, is_ad> & _eigenstrain;

  /**
   * Helper function for models that compute the eigenstrain based on a volumetric
   * strain.  This function computes the diagonal components of the eigenstrain tensor
   * as logarithmic strains.
   * @param volumetric_strain The current volumetric strain to be applied
   * @return Current strain in one direction due to volumetric strain, expressed as a logarithmic
   * strain
   */
  GenericReal<is_ad>
  computeVolumetricStrainComponent(const GenericReal<is_ad> & volumetric_strain) const;

  /// Restartable data to check for the zeroth and first time steps for thermal calculations
  bool & _step_zero;
};

typedef ComputeEigenstrainBaseTempl<false> ComputeEigenstrainBase;
typedef ComputeEigenstrainBaseTempl<true> ADComputeEigenstrainBase;
