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
 * ComputeEigenstrainInteractionBase is the base class for eigenstrain interaction tensors
 */
class ComputeEigenstrainInteractionBase : public Material
{
public:
  static InputParameters validParams();

  ComputeEigenstrainInteractionBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  ///Compute the eigenstrain interaction and store in _eigenstrainInteraction
  virtual void computeQpEigenstrainInteraction() = 0;

  ///Base name prepended to material property name
  const std::string _base_name;

  ///Material property name for the eigenstrain interaction tensor
  std::string _eigenstrainInteraction_name;

  ///Stores the current total eigenstrain interaction
  MaterialProperty<RankTwoTensor> & _eigenstrainInteraction;

  /**
   * Helper function for models that compute the eigenstrain interaction based on a volumetric
   * strain.  This function computes the diagonal components of the eigenstrain interaction tensor
   * as logarithmic strains.
   * @param volumetric_strain The current volumetric strain to be applied
   * @return Current strain in one direction due to volumetric strain, expressed as a logarithmic
   * strain
   */
  Real computeVolumetricStrainComponent(const Real volumetric_strain) const;

  /// Restartable data to check for the zeroth and first time steps for thermal calculations
  bool & _step_zero;
};
