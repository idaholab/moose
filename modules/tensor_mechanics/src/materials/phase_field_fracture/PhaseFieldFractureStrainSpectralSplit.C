//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldFractureStrainSpectralSplit.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", PhaseFieldFractureStrainSpectralSplit);

InputParameters
PhaseFieldFractureStrainSpectralSplit::validParams()
{
  InputParameters params = PhaseFieldFractureSpectralSplitBase::validParams();
  return params;
}

PhaseFieldFractureStrainSpectralSplit::PhaseFieldFractureStrainSpectralSplit(
    const InputParameters & parameters)
  : PhaseFieldFractureSpectralSplitBase(parameters),
    GuaranteeConsumer(this),
    _strain_pos(declareProperty<RankTwoTensor>(_base_name + "elastic_strain_positive")),
    _P_pos(declareProperty<RankFourTensor>(_base_name + "elastic_strain_positive_projector"))
{
}

void
PhaseFieldFractureStrainSpectralSplit::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_base_name + "elasticity_tensor", Guarantee::ISOTROPIC))
    mooseError(name(), " can only be used when the elasticity tensor is isotropic.");
}

void
PhaseFieldFractureStrainSpectralSplit::updateJacobianMultForDamage(RankFourTensor & jacobian_mult)
{
  if (_hybrid)
    jacobian_mult *= _g[_qp];
  else
  {
    const Real lambda = ElasticityTensorTools::getIsotropicLameConstant(_elasticity_tensor[_qp]);
    const Real mu = ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
    const Real strain_tr = _elastic_strain[_qp].trace();
    const Real dstrain_tr_pos_dstrain_tr = Macaulay(strain_tr, true);
    const RankTwoTensor I2(RankTwoTensor::initIdentity);
    const RankFourTensor I2I2 = I2.outerProduct(I2);

    const RankFourTensor jacobian_mult_pos =
        lambda * dstrain_tr_pos_dstrain_tr * I2I2 + 2 * mu * _P_pos[_qp];
    jacobian_mult -= (1 - _g[_qp]) * jacobian_mult_pos;
  }
}

void
PhaseFieldFractureStrainSpectralSplit::computeDamagedStress(RankTwoTensor & stress_new)
{
  const Real lambda = ElasticityTensorTools::getIsotropicLameConstant(_elasticity_tensor[_qp]);
  const Real mu = ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
  const Real strain_tr = _elastic_strain[_qp].trace();
  const Real strain_tr_pos = Macaulay(strain_tr);
  const RankTwoTensor I2(RankTwoTensor::initIdentity);

  spectralSplit(_elastic_strain[_qp], _strain_pos[_qp], _P_pos[_qp]);

  _stress_pos[_qp] = lambda * strain_tr_pos * I2 + 2 * mu * _strain_pos[_qp];

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
PhaseFieldFractureStrainSpectralSplit::computeElasticEnergy()
{
  const Real lambda = ElasticityTensorTools::getIsotropicLameConstant(_elasticity_tensor[_qp]);
  const Real mu = ElasticityTensorTools::getIsotropicShearModulus(_elasticity_tensor[_qp]);
  const Real strain_tr = _elastic_strain[_qp].trace();
  const Real strain_tr_pos = Macaulay(strain_tr);

  const Real E0 = 0.5 * lambda * _elastic_strain[_qp].trace() * _elastic_strain[_qp].trace() +
                  mu * _elastic_strain[_qp].doubleContraction(_elastic_strain[_qp]);
  _E_active[_qp] = 0.5 * lambda * strain_tr_pos * strain_tr_pos +
                   mu * _strain_pos[_qp].doubleContraction(_strain_pos[_qp]);
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
