//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMEquilibriumTractionCalculatorBase.h"
/**
 * This class uses the interface traction and its derivatives w.r.t. the interface displacment
 * jump to compute their respective values in global coordinatesusing a total Lagrangian
 * approach. The values computed by this object are used by the
 * CZMInterfaceKernelTotalLagrangian to add the proper residual to the system and to compute the
 * analytic Jacobian.
 */

class CZMEquilibriumTractionCalculatorTotalLagrangian : public CZMEquilibriumTractionCalculatorBase
{
public:
  static InputParameters validParams();
  CZMEquilibriumTractionCalculatorTotalLagrangian(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;

  /// computes the PK1 traction and its derivatives
  void computeEquilibriumTracionAndDerivatives() override final;

  /// computes the PK1 traction derivatives w.r.t. the global displacement jump
  void computedTPK1dJumpGlobal();

  /// computes the area ratio and increment rate derivatives
  void computeAreaRatioAndIncrementRateDerivatives();

  /// computes the PK1 traction derivatives w.r.t. F
  void assembledTPK1dF();

  /// the displacement jump, its increment, and its old value in global coordinates
  ///@{
  const MaterialProperty<RealVectorValue> & _displacement_jump_global;
  RealVectorValue _displacement_jump_global_inc;
  const MaterialProperty<RealVectorValue> & _displacement_jump_global_old;
  ///@}

  /// the interface traction increment, and old values
  ///@{
  RealVectorValue _interface_traction_inc;
  const MaterialProperty<RealVectorValue> & _interface_traction_old;
  ///@}

  /// the interface total and incremental rotation
  ///@{
  const MaterialProperty<RankTwoTensor> & _Q;
  const MaterialProperty<RankTwoTensor> & _DQ;
  ///@}

  /// the rotation associated to F
  const MaterialProperty<RankTwoTensor> & _R;

  /// the rotation derivatives w.r.t. F
  ///@{
  RankFourTensor _dR_dF;
  RankFourTensor _dQ_dF;
  ///@}

  /// the current, and old interface deformation gradient values
  ///@{
  const MaterialProperty<RankTwoTensor> & _F;
  const MaterialProperty<RankTwoTensor> & _F_old;
  ///@}

  /// the deformation gradient determinant;
  Real _J;

  /// the inverse of the deformation gradient
  RankTwoTensor _F_inv;

  /// the velocity gradient increment
  RankTwoTensor _DL;

  /// the PK1 traction values
  ///@{
  MaterialProperty<RealVectorValue> & _PK1traction;
  MaterialProperty<RealVectorValue> & _PK1traction_inc;
  const MaterialProperty<RealVectorValue> & _PK1traction_old;
  ///@}

  MaterialProperty<RankThreeTensor> & _dPK1traction_dF;

  /// the area raio and the area increment rate
  ///@{
  Real _area_ratio;
  Real _area_increment_rate;
  ///@}

  /// the area raio and the area increment rate derivatives w.r.t. F
  ///@{
  RankTwoTensor _d_area_ratio_dF;
  RankTwoTensor _d_area_increment_rate_dF;
  ///@}

  /// the midplane normal in the deformed configuration
  RealVectorValue _midplane_normal;
};
