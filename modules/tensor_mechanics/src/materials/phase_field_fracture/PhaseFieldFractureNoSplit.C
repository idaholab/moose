//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureNoSplit.h"

registerMooseObject("TensorMechanicsApp", PhaseFieldFractureNoSplit);

InputParameters
PhaseFieldFractureNoSplit::validParams()
{
  InputParameters params = PhaseFieldFractureBase::validParams();
  params.addClassDescription("The phase-field fracture model with an isotropic degradation.");
  return params;
}

PhaseFieldFractureNoSplit::PhaseFieldFractureNoSplit(const InputParameters & parameters)
  : PhaseFieldFractureBase(parameters)
{
}

void
PhaseFieldFractureNoSplit::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  jacobian_mult *= _g[_qp];
}

void
PhaseFieldFractureNoSplit::computeDamagedStress(RankTwoTensor & stress_new)
{
  _stress_pos[_qp] = stress_new;
  stress_new = _g[_qp] * _stress_pos[_qp];
  _dstress_dc[_qp] = _dg_dc[_qp] * _stress_pos[_qp];
}

void
PhaseFieldFractureNoSplit::computeElasticEnergy()
{
  _E_active[_qp] = 0.5 * _undamaged_stress[_qp].doubleContraction(_elastic_strain[_qp]);
  _E[_qp] = _g[_qp] * _E_active[_qp];

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
