//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureStrainVolDevSplit.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", PhaseFieldFractureStrainVolDevSplit);

InputParameters
PhaseFieldFractureStrainVolDevSplit::validParams()
{
  InputParameters params = PhaseFieldFractureVolDevSplitBase::validParams();
  return params;
}

PhaseFieldFractureStrainVolDevSplit::PhaseFieldFractureStrainVolDevSplit(
    const InputParameters & parameters)
  : PhaseFieldFractureVolDevSplitBase(parameters), GuaranteeConsumer(this)
{
}

void
PhaseFieldFractureStrainVolDevSplit::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_base_name + "elasticity_tensor", Guarantee::ISOTROPIC))
    mooseError(name(), " can only be used when the elasticity tensor is isotropic.");
}

void
PhaseFieldFractureStrainVolDevSplit::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  if (_hybrid)
    jacobian_mult *= _g[_qp];
  else
  {
    const Real K = ElasticityTensorTools::getIsotropicBulkModulus(_elasticity_tensor[_qp]);
    const Real strain_tr = _elastic_strain[_qp].trace();
    const Real dstrain_tr_neg_dstrain_tr = 1 - Macaulay(strain_tr, true);
    const RankTwoTensor I2(RankTwoTensor::initIdentity);
    const RankFourTensor I2I2 = I2.outerProduct(I2);

    const RankFourTensor jacobian_mult_neg = K * dstrain_tr_neg_dstrain_tr * I2I2;
    jacobian_mult = _g[_qp] * (jacobian_mult - jacobian_mult_neg) + jacobian_mult_neg;
  }
}

void
PhaseFieldFractureStrainVolDevSplit::computeDamagedStress(RankTwoTensor & stress_new)
{
  if (_hybrid)
  {
    stress_new *= _g[_qp];
    _dstress_dc[_qp] = _dg_dc[_qp] * stress_new;
  }
  else
  {
    const Real K = ElasticityTensorTools::getIsotropicBulkModulus(_elasticity_tensor[_qp]);
    const RankTwoTensor I2(RankTwoTensor::initIdentity);

    // Vol-dev decomposition
    const Real strain_tr = _elastic_strain[_qp].trace();
    const Real strain_tr_pos = Macaulay(strain_tr);
    const Real strain_tr_neg = strain_tr - strain_tr_pos;
    const RankTwoTensor strain_dev = _elastic_strain[_qp].deviatoric();
    const RankTwoTensor stress_neg = K * strain_tr_neg * I2;
    const RankTwoTensor stress_pos = stress_new - stress_neg;

    stress_new = _g[_qp] * stress_pos + stress_neg;
    _dstress_dc[_qp] = _dg_dc[_qp] * stress_pos;
  }
}

void
PhaseFieldFractureStrainVolDevSplit::computeElasticEnergy()
{
  const Real K = ElasticityTensorTools::getIsotropicBulkModulus(_elasticity_tensor[_qp]);
  const Real mu = ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
  const Real strain_tr = _elastic_strain[_qp].trace();
  const Real strain_tr_pos = Macaulay(strain_tr);
  const RankTwoTensor strain_dev = _elastic_strain[_qp].deviatoric();

  const Real E0 = 0.5 * K * strain_tr * strain_tr + mu * strain_dev.doubleContraction(strain_dev);
  _E_active[_qp] =
      0.5 * K * strain_tr_pos * strain_tr_pos + mu * strain_dev.doubleContraction(strain_dev);
  _E[_qp] = E0 - (1 - _g[_qp]) * _E_active[_qp];

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
