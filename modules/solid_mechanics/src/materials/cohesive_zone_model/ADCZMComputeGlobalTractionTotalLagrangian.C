//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCZMComputeGlobalTractionTotalLagrangian.h"
#include "CZMComputeGlobalTractionTotalLagrangian.h"

registerMooseObject("TensorMechanicsApp", ADCZMComputeGlobalTractionTotalLagrangian);

InputParameters
ADCZMComputeGlobalTractionTotalLagrangian::validParams()
{
  InputParameters params = CZMComputeGlobalTractionTotalLagrangian::validParams();
  return params;
}

ADCZMComputeGlobalTractionTotalLagrangian::ADCZMComputeGlobalTractionTotalLagrangian(
    const InputParameters & parameters)
  : ADCZMComputeGlobalTractionBase(parameters),
    _displacement_jump_global(
        getADMaterialPropertyByName<RealVectorValue>(_base_name + "displacement_jump_global")),
    _czm_reference_rotation(
        getADMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_reference_rotation")),
    _F(getADMaterialPropertyByName<RankTwoTensor>(_base_name + "F_czm")),
    _R(getADMaterialPropertyByName<RankTwoTensor>(_base_name + "czm_rotation")),
    _PK1traction(declareADPropertyByName<RealVectorValue>(_base_name + "PK1traction"))
{
}

void
ADCZMComputeGlobalTractionTotalLagrangian::initQpStatefulProperties()
{
  _PK1traction[_qp] = 0;
}

void
ADCZMComputeGlobalTractionTotalLagrangian::computeEquilibriumTracion()
{
  const auto J = _F[_qp].det();
  const auto F_inv = _F[_qp].inverse();
  const auto area_ratio = J * (F_inv.transpose() * _normals[_qp]).norm();

  _traction_global[_qp] = _czm_total_rotation[_qp] * _interface_traction[_qp];
  _PK1traction[_qp] = _traction_global[_qp] * area_ratio;
}
