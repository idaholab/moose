//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputePFFractureStressBase.h"
#include "MooseEnum.h"
#include "GuaranteeConsumer.h"

/**
 * Phase-field fracture
 * This class computes the stress and energy contribution for the
 * small strain Linear Elastic formulation of phase field fracture
 */
class ComputeLinearElasticPFFractureStress : public ComputePFFractureStressBase,
                                             public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  ComputeLinearElasticPFFractureStress(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /**
   * Method to split elastic energy based on strain spectral decomposition
   * @param F_pos tensile part of total elastic energy
   * @param F_neg compressive part of total elastic energy
   */
  void computeStrainSpectral(Real & F_pos, Real & F_neg);

  /**
   * Method to split elastic energy based on strain volumetric/deviatoric decomposition
   * @param F_pos tensile part of total elastic energy
   * @param F_neg compressive part of total elastic energy
   */
  void computeStrainVolDev(Real & F_pos, Real & F_neg);

  /**
   * Method to split elastic energy based on stress spectral decomposition
   * @param F_pos tensile part of total elastic energy
   * @param F_neg compressive part of total elastic energy
   */
  void computeStressSpectral(Real & F_pos, Real & F_neg);

  /// Decomposittion type
  enum class Decomposition_type
  {
    strain_spectral,
    strain_vol_dev,
    stress_spectral,
    none
  } _decomposition_type;
};
