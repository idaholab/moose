//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Material Description of Slip Weakening Friction 3d
*/

#pragma once

#include "CZMComputeLocalTractionTotalBase.h"

class SlipWeakeningFriction3d : public CZMComputeLocalTractionTotalBase
{
public:
  static InputParameters validParams();
  SlipWeakeningFriction3d(const InputParameters & parameters);

protected:
  /// method computing the total traction and its derivatives
  void computeInterfaceTractionAndDerivatives() override;

  Real _T2_o;
  Real _T3_o;
  Real _mu_d;
  Real _Dc;
  Real _len;

  const MaterialProperty<Real> & _density;

  const MaterialProperty<RankTwoTensor> & _rot;

  const VariableValue & _disp_slipweakening_x;
  const VariableValue & _disp_slipweakening_neighbor_x;
  const VariableValue & _disp_slipweakening_y;
  const VariableValue & _disp_slipweakening_neighbor_y;
  const VariableValue & _disp_slipweakening_z;
  const VariableValue & _disp_slipweakening_neighbor_z;

  const VariableValue & _reaction_slipweakening_x;
  const VariableValue & _reaction_slipweakening_neighbor_x;
  const VariableValue & _reaction_slipweakening_y;
  const VariableValue & _reaction_slipweakening_neighbor_y;
  const VariableValue & _reaction_slipweakening_z;
  const VariableValue & _reaction_slipweakening_neighbor_z;

  const VariableValue & _disp_slipweakening_x_old;
  const VariableValue & _disp_slipweakening_neighbor_x_old;
  const VariableValue & _disp_slipweakening_y_old;
  const VariableValue & _disp_slipweakening_neighbor_y_old;
  const VariableValue & _disp_slipweakening_z_old;
  const VariableValue & _disp_slipweakening_neighbor_z_old;

  const VariableValue & _mu_s;
  const VariableValue & _ini_shear_sts;
};
