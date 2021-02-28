//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMEquilibriumTractionCalculatorTotalLagrangian.h"
#include "DeformationGradientTools.h"

registerMooseObject("TensorMechanicsApp", CZMEquilibriumTractionCalculatorTotalLagrangian);

InputParameters
CZMEquilibriumTractionCalculatorTotalLagrangian::validParams()
{
  InputParameters params = CZMEquilibriumTractionCalculatorBase::validParams();

  params.addClassDescription("Compute the equilibrium traction (PK1) and its derivatives for the "
                             "Total Lagrangian formulation.");
  return params;
}

CZMEquilibriumTractionCalculatorTotalLagrangian::CZMEquilibriumTractionCalculatorTotalLagrangian(
    const InputParameters & parameters)
  : CZMEquilibriumTractionCalculatorBase(parameters),
    _displacement_jump_global(getMaterialProperty<RealVectorValue>("displacement_jump_global")),
    _displacement_jump_global_old(
        getMaterialPropertyOld<RealVectorValue>("displacement_jump_global")),
    _interface_traction_old(getMaterialPropertyOld<RealVectorValue>("interface_traction_old")),
    _Q(getMaterialProperty<RankTwoTensor>("czm_total_rotation")),
    _DQ(getMaterialProperty<RankTwoTensor>("czm_total_rotation_inc")),
    _R(getMaterialProperty<RankTwoTensor>("czm_rotation")),
    _F(getMaterialProperty<RankTwoTensor>("F_czm")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>("F_czm")),
    _PK1traction(declareProperty<RealVectorValue>("PK1traction")),
    _PK1traction_inc(declareProperty<RealVectorValue>("PK1traction_inc")),
    _PK1traction_old(getMaterialPropertyOld<RealVectorValue>("PK1traction")),
    _dPK1traction_dF(declareProperty<RankThreeTensor>("dPK1traction_dF"))
{
}

void
CZMEquilibriumTractionCalculatorTotalLagrangian::initQpStatefulProperties()
{
  _PK1traction[_qp] = 0;
}

void
CZMEquilibriumTractionCalculatorTotalLagrangian::computeEquilibriumTracionAndDerivatives()
{
  _displacement_jump_global_inc =
      _displacement_jump_global[_qp] - _displacement_jump_global_old[_qp];
  _interface_traction_inc = _interface_traction[_qp] - _interface_traction_old[_qp];

  _J = _F[_qp].det();
  _F_inv = _F[_qp].inverse();
  _midplane_normal = _R[_qp] * _normals[_qp];

  _DL = DeformationGradientTools::computeVelocityGradientLinearApprox(_F[_qp], _F_old[_qp]);

  _area_increment_rate =
      DeformationGradientTools::computeAreaIncrementRate(_DL.trace(), _DL, _midplane_normal);
  _area_ratio = DeformationGradientTools::computeAreaRatio(_F_inv.transpose(), _J, _normals[_qp]);

  // assemble PK1 traction
  _PK1traction_inc[_qp] =
      (_area_ratio * (_DQ[_qp] * _interface_traction[_qp] + _Q[_qp] * _interface_traction_inc) +
       _area_increment_rate * _PK1traction_old[_qp]) /
      (1. - _area_increment_rate);

  _PK1traction[_qp] = _PK1traction_old[_qp] + _PK1traction_inc[_qp];
  _traction_global[_qp] = _PK1traction[_qp] / _area_ratio;

  // compute derivatives of _PK1traction wrt dF
  computedTPK1dJumpGlobal();
  computeAreaRatioAndIncrementRateDerivatives();
  assembledTPK1dF();
}

void
CZMEquilibriumTractionCalculatorTotalLagrangian::computedTPK1dJumpGlobal()
{

  // compute the PK1 traction derivatives w.r.t the displacment jump in global
  // coordinates
  const RankTwoTensor djump_djumpglobal = (_DQ[_qp] + _Q[_qp]).transpose();
  const RankTwoTensor dinterface_traction_djumpglobal =
      _dinterface_traction_djump[_qp] * djump_djumpglobal;
  _dtraction_djump_global[_qp] = _area_ratio * (_DQ[_qp] + _Q[_qp]) *
                                 dinterface_traction_djumpglobal / (1. - _area_increment_rate);
}

void
CZMEquilibriumTractionCalculatorTotalLagrangian::computeAreaRatioAndIncrementRateDerivatives()
{
  _dR_dF = DeformationGradientTools::computedRdF(_R[_qp], _R[_qp].transpose() * _F[_qp]);
  _dQ_dF = AdditionalTensorTools::R4ijklR2jm(_dR_dF, _Q0[_qp]);

  const RankTwoTensor F_itr = _F_inv.transpose();
  const RealVectorValue Fitr_N = F_itr * _normals[_qp];
  const RankFourTensor dFinv_dF = DeformationGradientTools::computedFinversedF(_F_inv);
  const RankFourTensor dDL_dF = DeformationGradientTools::computeDL_DF(dFinv_dF, _F_old[_qp]);
  const RankTwoTensor dDLtrace_DF = DeformationGradientTools::computeDtraceL_DF(dDL_dF);
  _d_area_ratio_dF =
      DeformationGradientTools::computeDAreaRatioDF(F_itr, _normals[_qp], _J, dFinv_dF);
  _d_area_increment_rate_dF = DeformationGradientTools::computeDAreaIncrementRateDF(
      _DL, dDLtrace_DF, dDL_dF, _normals[_qp], _R[_qp], _dR_dF);
}

void
CZMEquilibriumTractionCalculatorTotalLagrangian::assembledTPK1dF()
{
  // The derivative of the PK1 traction w.r.t. F
  const RealVectorValue global_traction_incremenet =
      _DQ[_qp] * _interface_traction[_qp] + _Q[_qp] * _interface_traction_inc;
  const RankTwoTensor dglobal_traction_inc_djump =
      (_DQ[_qp] + _Q[_qp]) * _dinterface_traction_djump[_qp];
  const RankThreeTensor djump_dF = AdditionalTensorTools::R4ijklVj(
      AdditionalTensorTools::R4ijklSwapij(_dQ_dF),
      _displacement_jump_global[_qp] + _displacement_jump_global_inc);

  _dPK1traction_dF[_qp] =
      AdditionalTensorTools::R2jkVi(_d_area_ratio_dF, global_traction_incremenet);
  _dPK1traction_dF[_qp] +=
      (_area_ratio *
       AdditionalTensorTools::R4ijklVj(_dQ_dF, _interface_traction[_qp] + _interface_traction_inc));
  _dPK1traction_dF[_qp] +=
      _area_ratio * AdditionalTensorTools::R2ijR3jkl(dglobal_traction_inc_djump, djump_dF);
  _dPK1traction_dF[_qp] /= (1 - _area_increment_rate);
  _dPK1traction_dF[_qp] +=
      (AdditionalTensorTools::R2jkVi(
           _d_area_increment_rate_dF,
           (_area_ratio * global_traction_incremenet + _PK1traction_old[_qp])) /
       ((1. - _area_increment_rate) * (1. - _area_increment_rate)));
}
