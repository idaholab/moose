//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeGlobalTractionTotalLagrangian.h"
#include "CohesiveZoneModelTools.h"

registerMooseObject("TensorMechanicsApp", CZMComputeGlobalTractionTotalLagrangian);

InputParameters
CZMComputeGlobalTractionTotalLagrangian::validParams()
{
  InputParameters params = CZMComputeGlobalTractionBase::validParams();

  params.addClassDescription("Compute the equilibrium traction (PK1) and its derivatives for the "
                             "Total Lagrangian formulation.");
  return params;
}

CZMComputeGlobalTractionTotalLagrangian::CZMComputeGlobalTractionTotalLagrangian(
    const InputParameters & parameters)
  : CZMComputeGlobalTractionBase(parameters),
    _displacement_jump_global(
        getMaterialProperty<RealVectorValue>(_base_name + "displacement_jump_global")),
    _displacement_jump_global_old(
        getMaterialPropertyOld<RealVectorValue>(_base_name + "displacement_jump_global")),
    _interface_traction_old(
        getMaterialPropertyOld<RealVectorValue>(_base_name + "interface_traction_old")),
    _czm_total_rotation(getMaterialProperty<RankTwoTensor>(_base_name + "czm_total_rotation")),
    _czm_total_rotation_inc(
        getMaterialProperty<RankTwoTensor>(_base_name + "czm_total_rotation_inc")),
    _R(getMaterialProperty<RankTwoTensor>(_base_name + "czm_rotation")),
    _F(getMaterialProperty<RankTwoTensor>(_base_name + "F_czm")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "F_czm")),
    _PK1traction(declareProperty<RealVectorValue>(_base_name + "PK1traction")),
    _PK1traction_inc(declareProperty<RealVectorValue>(_base_name + "PK1traction_inc")),
    _PK1traction_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "PK1traction")),
    _dPK1traction_dF(declareProperty<RankThreeTensor>(_base_name + "dPK1traction_dF"))
{
}

void
CZMComputeGlobalTractionTotalLagrangian::initQpStatefulProperties()
{
  _PK1traction[_qp] = 0;
}

void
CZMComputeGlobalTractionTotalLagrangian::computeEquilibriumTracionAndDerivatives()
{
  _displacement_jump_global_inc =
      _displacement_jump_global[_qp] - _displacement_jump_global_old[_qp];
  _interface_traction_inc = _interface_traction[_qp] - _interface_traction_old[_qp];

  _J = _F[_qp].det();
  _F_inv = _F[_qp].inverse();
  _midplane_normal = _R[_qp] * _normals[_qp];

  _DL = CohesiveZoneModelTools::computeVelocityGradientLinearApprox(_F[_qp], _F_old[_qp]);

  _area_increment_rate =
      CohesiveZoneModelTools::computeAreaIncrementRate(_DL.trace(), _DL, _midplane_normal);
  _area_ratio = CohesiveZoneModelTools::computeAreaRatio(_F_inv.transpose(), _J, _normals[_qp]);

  // assemble PK1 traction
  _PK1traction_inc[_qp] = (_area_ratio * (_czm_total_rotation_inc[_qp] * _interface_traction[_qp] +
                                          _czm_total_rotation[_qp] * _interface_traction_inc) +
                           _area_increment_rate * _PK1traction_old[_qp]) /
                          (1. - _area_increment_rate);

  _PK1traction[_qp] = _PK1traction_old[_qp] + _PK1traction_inc[_qp];
  _traction_global[_qp] = _PK1traction[_qp] / _area_ratio;

  // compute derivatives of _PK1traction wrt dF
  computedTPK1dJumpGlobal();
  computeAreaRatioAndIncrementRateDerivatives();
  computedTPK1dF();
}

void
CZMComputeGlobalTractionTotalLagrangian::computedTPK1dJumpGlobal()
{

  // compute the PK1 traction derivatives w.r.t the displacment jump in global
  // coordinates
  const RankTwoTensor djump_djumpglobal =
      (_czm_total_rotation_inc[_qp] + _czm_total_rotation[_qp]).transpose();
  const RankTwoTensor dinterface_traction_djumpglobal =
      _dinterface_traction_djump[_qp] * djump_djumpglobal;
  _dtraction_djump_global[_qp] = _area_ratio *
                                 (_czm_total_rotation_inc[_qp] + _czm_total_rotation[_qp]) *
                                 dinterface_traction_djumpglobal / (1. - _area_increment_rate);
}

void
CZMComputeGlobalTractionTotalLagrangian::computeAreaRatioAndIncrementRateDerivatives()
{
  _dR_dF = CohesiveZoneModelTools::computedRdF(_R[_qp], _R[_qp].transpose() * _F[_qp]);
  _dczm_total_rotation_dF = _czm_reference_rotation[_qp].mixedProductJkIjlm(_dR_dF);

  const RankTwoTensor F_itr = _F_inv.transpose();
  const RealVectorValue Fitr_N = F_itr * _normals[_qp];
  const RankFourTensor dFinv_dF = CohesiveZoneModelTools::computedFinversedF(_F_inv);
  const RankFourTensor dDL_dF = CohesiveZoneModelTools::computeDL_DF(dFinv_dF, _F_old[_qp]);
  const RankTwoTensor dDLtrace_DF = CohesiveZoneModelTools::computeDtraceL_DF(dDL_dF);
  _d_area_ratio_dF =
      CohesiveZoneModelTools::computeDAreaRatioDF(F_itr, _normals[_qp], _J, dFinv_dF);
  _d_area_increment_rate_dF = CohesiveZoneModelTools::computeDAreaIncrementRateDF(
      _DL, dDLtrace_DF, dDL_dF, _normals[_qp], _R[_qp], _dR_dF);
}

void
CZMComputeGlobalTractionTotalLagrangian::computedTPK1dF()
{
  // The derivative of the PK1 traction w.r.t. F
  const RealVectorValue global_traction_incremenet =
      _czm_total_rotation_inc[_qp] * _interface_traction[_qp] +
      _czm_total_rotation[_qp] * _interface_traction_inc;
  const RankTwoTensor dglobal_traction_inc_djump =
      (_czm_total_rotation_inc[_qp] + _czm_total_rotation[_qp]) * _dinterface_traction_djump[_qp];
  const RankThreeTensor djump_dF = _dczm_total_rotation_dF.transposeIj().mixedProductIjklJ(
      _displacement_jump_global[_qp] + _displacement_jump_global_inc);

  _dPK1traction_dF[_qp] = _d_area_ratio_dF.mixedProductJkI(global_traction_incremenet);
  _dPK1traction_dF[_qp] += (_area_ratio * _dczm_total_rotation_dF.mixedProductIjklJ(
                                              _interface_traction[_qp] + _interface_traction_inc));
  _dPK1traction_dF[_qp] += _area_ratio * dglobal_traction_inc_djump.mixedProductIjJkl(djump_dF);
  _dPK1traction_dF[_qp] /= (1. - _area_increment_rate);
  _dPK1traction_dF[_qp] += (_d_area_increment_rate_dF.mixedProductJkI(
                                _area_ratio * global_traction_incremenet + _PK1traction_old[_qp]) /
                            ((1. - _area_increment_rate) * (1. - _area_increment_rate)));
}
