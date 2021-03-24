//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureStressSpectralSplit.h"

registerMooseObject("TensorMechanicsApp", PhaseFieldFractureStressSpectralSplit);

InputParameters
PhaseFieldFractureStressSpectralSplit::validParams()
{
  InputParameters params = PhaseFieldFractureSpectralSplitBase::validParams();
  return params;
}

PhaseFieldFractureStressSpectralSplit::PhaseFieldFractureStressSpectralSplit(
    const InputParameters & parameters)
  : PhaseFieldFractureSpectralSplitBase(parameters),
    _P_pos(declareProperty<RankFourTensor>(_base_name + "stress_positive_projector"))
{
}

void
PhaseFieldFractureStressSpectralSplit::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  if (_hybrid)
    jacobian_mult *= _g[_qp];
  else
    jacobian_mult -= (1. - _g[_qp]) * _P_pos[_qp] * _elasticity_tensor[_qp];
}

void
PhaseFieldFractureStressSpectralSplit::computeDamagedStress(RankTwoTensor & stress_new)
{
  RankTwoTensor I2(RankTwoTensor::initIdentity);

  spectralSplit(stress_new, _stress_pos[_qp], _P_pos[_qp]);

  if (_hybrid)
  {
    stress_new *= _g[_qp];
    _dstress_dc[_qp] = _dg_dc[_qp] * stress_new;
  }
  else
  {
    stress_new -= (1 - _g[_qp]) * _stress_pos[_qp];
    _dstress_dc[_qp] = _dg_dc[_qp] * _stress_pos[_qp];
  }
}

void
PhaseFieldFractureStressSpectralSplit::computeElasticEnergy()
{
  Real E0 = 0.5 * _undamaged_stress[_qp].doubleContraction(_elastic_strain[_qp]);
  _E_active[_qp] = 0.5 * _stress_pos[_qp].doubleContraction(_elastic_strain[_qp]);
  _E[_qp] = E0 - (1. - _g[_qp]) * _E_active[_qp];

  if (_use_old_elastic_energy)
  {
    _dE_dc[_qp] = _dg_dc[_qp] * _E_active_old[_qp];
    _d2E_dc2[_qp] = 0;
    _d2E_dcdstrain[_qp].zero();
  }
  else
  {
    _dE_dc[_qp] = _dg_dc[_qp] * _E_active[_qp];
    _d2E_dc2[_qp] = _d2g_dc2[_qp] * _E_active[_qp];
    _d2E_dcdstrain[_qp] = _dg_dc[_qp] * _stress_pos[_qp];
  }
}
