//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCZMComputeGlobalTractionBase.h"

/**
 * AD equivalent of CZMComputeGlobalTractionTotalLagrangian
 */
class ADCZMComputeGlobalTractionTotalLagrangian : public ADCZMComputeGlobalTractionBase
{
public:
  static InputParameters validParams();
  ADCZMComputeGlobalTractionTotalLagrangian(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  void computeEquilibriumTracion() override;

  /// computes the area ratio
  void computeAreaRatio();

  /// the displacement jump in global coordinates
  const ADMaterialProperty<RealVectorValue> & _displacement_jump_global;

  /// the rotation matrix transforming from local to global coordinates in the undeformed configuration
  const ADMaterialProperty<RankTwoTensor> & _czm_reference_rotation;

  /// the interface deformation gradient
  const ADMaterialProperty<RankTwoTensor> & _F;

  /// the rotation associated to F
  const ADMaterialProperty<RankTwoTensor> & _R;

  /// the PK1 traction
  ADMaterialProperty<RealVectorValue> & _PK1traction;
};
