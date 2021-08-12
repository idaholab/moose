//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeGlobalTractionBase.h"
/**
 * This class uses the interface traction and its derivatives w.r.t. the interface displacment
 * jump to compute their respective values in global coordinatesusing a total Lagrangian
 * approach. The values computed by this object are used by the
 * CZMInterfaceKernelTotalLagrangian to add the proper residual to the system and to compute the
 * analytic Jacobian.
 */

class CZMComputeGlobalTractionTotalLagrangian : public CZMComputeGlobalTractionBase
{
public:
  static InputParameters validParams();
  CZMComputeGlobalTractionTotalLagrangian(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// computes the PK1 traction and its derivatives
  void computeEquilibriumTracionAndDerivatives() override;

  /// computes the PK1 traction derivatives w.r.t. the global displacement jump
  void computedTPK1dJumpGlobal();

  /// computes the area ratio and increment rate derivatives
  void computeAreaRatioAndDerivatives();

  /// computes the PK1 traction derivatives w.r.t. F
  void computedTPK1dF();

  /// the displacement jump in global coordinates
  const MaterialProperty<RealVectorValue> & _displacement_jump_global;

  /// the rotation matrix transforming from local to global coordinates in the undeformed configuration
  const MaterialProperty<RankTwoTensor> & _czm_reference_rotation;

  /// the rotation associated to F
  const MaterialProperty<RankTwoTensor> & _R;

  /// the rotation derivatives w.r.t. F
  ///@{
  RankFourTensor _dR_dF;
  RankFourTensor _dczm_total_rotation_dF;
  ///@}

  /// the interface deformation gradient
  const MaterialProperty<RankTwoTensor> & _F;

  /// the deformation gradient determinant;
  Real _J;

  /// the inverse of the deformation gradient
  RankTwoTensor _F_inv;

  /// the PK1 traction
  MaterialProperty<RealVectorValue> & _PK1traction;

  /// the derivitve of the PK1 traction w.r.t. F
  MaterialProperty<RankThreeTensor> & _dPK1traction_dF;

  /// the area ratio and its derivtive w.r.t. F
  ///@{
  Real _area_ratio;
  RankTwoTensor _d_area_ratio_dF;
  ///@}
};
