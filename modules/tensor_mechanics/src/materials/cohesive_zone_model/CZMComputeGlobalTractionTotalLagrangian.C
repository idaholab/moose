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
        getMaterialPropertyByName<RealVectorValue>(_base_name + "displacement_jump_global")),
    _czm_reference_rotation(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_reference_rotation")),
    _R(getMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_rotation")),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "F_czm")),
    _PK1traction(declarePropertyByName<RealVectorValue>(_base_name + "PK1traction")),
    _dPK1traction_dF(declarePropertyByName<RankThreeTensor>(_base_name + "dPK1traction_dF"))
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
  _J = _F[_qp].det();
  _F_inv = _F[_qp].inverse();

  _area_ratio = CohesiveZoneModelTools::computeAreaRatio(_F_inv.transpose(), _J, _normals[_qp]);

  _traction_global[_qp] = (_czm_total_rotation[_qp] * _interface_traction[_qp]);
  _PK1traction[_qp] = _traction_global[_qp] * _area_ratio;

  // compute derivatives of _PK1traction wrt dF
  computedTPK1dJumpGlobal();
  computeAreaRatioAndDerivatives();
  computedTPK1dF();
}

void
CZMComputeGlobalTractionTotalLagrangian::computedTPK1dJumpGlobal()
{
  // compute the PK1 traction derivatives w.r.t the displacment jump in global
  // coordinates
  _dtraction_djump_global[_qp] = _area_ratio * _czm_total_rotation[_qp] *
                                 _dinterface_traction_djump[_qp] *
                                 _czm_total_rotation[_qp].transpose();
}

void
CZMComputeGlobalTractionTotalLagrangian::computeAreaRatioAndDerivatives()
{
  _dR_dF = CohesiveZoneModelTools::computedRdF(_R[_qp], _R[_qp].transpose() * _F[_qp]);
  usingTensorIndices(i_, j_, k_, l_, m_);
  _dczm_total_rotation_dF = _czm_reference_rotation[_qp].times<m_, j_, i_, m_, k_, l_>(_dR_dF);

  const RankFourTensor dFinv_dF = CohesiveZoneModelTools::computedFinversedF(_F_inv);

  _d_area_ratio_dF =
      CohesiveZoneModelTools::computeDAreaRatioDF(_F_inv.transpose(), _normals[_qp], _J, dFinv_dF);
}

void
CZMComputeGlobalTractionTotalLagrangian::computedTPK1dF()
{
  // The derivative of the PK1 traction w.r.t. F
  usingTensorIndices(i_, j_, k_, l_);
  const RankThreeTensor djump_dF =
      _dczm_total_rotation_dF.transposeIj().contraction<j_>(_displacement_jump_global[_qp]);

  _dPK1traction_dF[_qp] =
      _d_area_ratio_dF.mixedProductJkI(_czm_total_rotation[_qp] * _interface_traction[_qp]) +
      _area_ratio *
          (_dczm_total_rotation_dF.contraction<j_>(_interface_traction[_qp]) +
           (_czm_total_rotation[_qp] * _dinterface_traction_djump[_qp]).contraction(djump_dF));
}
